import { useCallback, useEffect, useRef, useState } from "react"

import { Sparkline } from "@/components/Sparkline"
import { Badge } from "@/components/ui/badge"
import { Button } from "@/components/ui/button"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { getStatus, ledOff, ledOn, ledToggle, type Status } from "@/lib/api"

const POLL_MS = 1000
const HISTORY = 60 // one minute of samples

function formatUptime(ms: number) {
  const s = Math.floor(ms / 1000)
  const d = Math.floor(s / 86400)
  const h = Math.floor((s % 86400) / 3600)
  const m = Math.floor((s % 3600) / 60)
  if (d > 0) return `${d}d ${h}h`
  if (h > 0) return `${h}h ${m}m`
  return `${m}m ${s % 60}s`
}

function formatBytes(b: number) {
  return b >= 1024 * 1024
    ? `${(b / 1024 / 1024).toFixed(2)} MB`
    : `${Math.round(b / 1024)} KB`
}

// -90 dBm is unusable, -30 is excellent.
function signalLabel(rssi: number) {
  if (rssi >= -55) return "excellent"
  if (rssi >= -67) return "good"
  if (rssi >= -80) return "fair"
  return "weak"
}

function Stat({
  label,
  value,
  sub,
  history,
}: {
  label: string
  value: string
  sub?: string
  history?: number[]
}) {
  return (
    <Card>
      <CardHeader className="pb-2">
        <CardTitle className="text-sm font-medium text-muted-foreground">
          {label}
        </CardTitle>
      </CardHeader>
      <CardContent>
        <div className="text-2xl font-semibold tabular-nums">{value}</div>
        {sub && <div className="mt-1 text-xs text-muted-foreground">{sub}</div>}
        {history && (
          <Sparkline values={history} className="mt-3 text-primary/70" />
        )}
      </CardContent>
    </Card>
  )
}

export default function App() {
  const [status, setStatus] = useState<Status | null>(null)
  const [error, setError] = useState<string | null>(null)
  const [busy, setBusy] = useState(false)
  const [rssiHistory, setRssiHistory] = useState<number[]>([])
  const [tempHistory, setTempHistory] = useState<number[]>([])

  // Kept in a ref so the poll loop never restarts when a command lands.
  const inFlight = useRef(false)

  const apply = useCallback((s: Status) => {
    setStatus(s)
    setError(null)
    setRssiHistory((h) => [...h, s.rssi].slice(-HISTORY))
    setTempHistory((h) => [...h, s.temp_c].slice(-HISTORY))
  }, [])

  useEffect(() => {
    let cancelled = false

    const poll = async () => {
      // The board serves one request at a time; overlapping polls would queue
      // up behind each other and make the UI feel worse, not better.
      if (inFlight.current) return
      inFlight.current = true
      try {
        const s = await getStatus()
        if (!cancelled) apply(s)
      } catch (e) {
        if (!cancelled) setError(e instanceof Error ? e.message : String(e))
      } finally {
        inFlight.current = false
      }
    }

    poll()
    const id = setInterval(poll, POLL_MS)
    return () => {
      cancelled = true
      clearInterval(id)
    }
  }, [apply])

  const command = async (fn: () => Promise<Status>) => {
    setBusy(true)
    try {
      apply(await fn())
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e))
    } finally {
      setBusy(false)
    }
  }

  const online = status !== null && error === null

  return (
    <div className="min-h-screen bg-background p-6">
      <div className="mx-auto max-w-4xl space-y-6">
        <header className="flex flex-wrap items-center justify-between gap-3">
          <div>
            <h1 className="text-2xl font-semibold tracking-tight">
              esp32-iot-telemetry
            </h1>
            <p className="text-sm text-muted-foreground">
              ESP32-S3 &middot; live device telemetry
            </p>
          </div>
          <Badge variant={online ? "default" : "destructive"}>
            {online ? "online" : "unreachable"}
          </Badge>
        </header>

        {error && (
          <Card className="border-destructive/40">
            <CardContent className="py-4 text-sm text-destructive">
              {error}
            </CardContent>
          </Card>
        )}

        <section className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
          <Stat
            label="LED"
            value={status?.led ? "on" : "off"}
            sub={
              status
                ? `${status.toggles.toLocaleString()} toggle${status.toggles === 1 ? "" : "s"}`
                : undefined
            }
          />
          <Stat
            label="Signal"
            value={status ? `${status.rssi} dBm` : "—"}
            sub={status ? signalLabel(status.rssi) : undefined}
            history={rssiHistory}
          />
          <Stat
            label="Chip temperature"
            value={status ? `${status.temp_c.toFixed(1)} °C` : "—"}
            sub="internal sensor"
            history={tempHistory}
          />
          <Stat
            label="Free heap"
            value={status ? formatBytes(status.free_heap) : "—"}
          />
          <Stat
            label="Uptime"
            value={status ? formatUptime(status.uptime_ms) : "—"}
          />
          <Stat
            label="Toggles"
            value={status ? status.toggles.toLocaleString() : "—"}
            sub="button and HTTP combined"
          />
        </section>

        <Card>
          <CardHeader>
            <CardTitle className="text-base">Control</CardTitle>
          </CardHeader>
          <CardContent className="flex flex-wrap gap-2">
            <Button onClick={() => command(ledOn)} disabled={busy}>
              On
            </Button>
            <Button
              variant="secondary"
              onClick={() => command(ledOff)}
              disabled={busy}
            >
              Off
            </Button>
            <Button
              variant="outline"
              onClick={() => command(ledToggle)}
              disabled={busy}
            >
              Toggle
            </Button>
          </CardContent>
        </Card>

        <p className="text-center text-xs text-muted-foreground">
          The physical button toggles the same LED — presses show up here within
          a second.
        </p>
      </div>
    </div>
  )
}
