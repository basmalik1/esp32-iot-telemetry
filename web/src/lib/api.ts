// In production this bundle is served by the board itself, so relative paths
// hit the right host. During `npm run dev` it is served by Vite on localhost,
// so point it at a real board with:
//   VITE_BOARD_URL=http://10.0.0.42 npm run dev
const BASE = import.meta.env.VITE_BOARD_URL ?? ""

export type Status = {
  led: boolean
  toggles: number
  rssi: number
  uptime_ms: number
  free_heap: number
  temp_c: number
}

async function request(path: string): Promise<Status> {
  // The board runs a single-threaded server; a stuck request would otherwise
  // hang the poll loop indefinitely.
  const controller = new AbortController()
  const timer = setTimeout(() => controller.abort(), 4000)
  try {
    const res = await fetch(`${BASE}${path}`, { signal: controller.signal })
    if (!res.ok) throw new Error(`${path} returned ${res.status}`)
    return (await res.json()) as Status
  } finally {
    clearTimeout(timer)
  }
}

export const getStatus = () => request("/status")
export const ledOn = () => request("/on")
export const ledOff = () => request("/off")
export const ledToggle = () => request("/toggle")
