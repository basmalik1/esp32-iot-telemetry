type Props = {
  values: number[]
  className?: string
}

// A hand-rolled SVG polyline rather than a charting library. Recharts and
// friends would add roughly 100 kB gzipped to a bundle that has to be stored
// in flash and served by a single-threaded web server on the device itself -
// a poor trade for what is ultimately a line with no axes.
export function Sparkline({ values, className }: Props) {
  if (values.length < 2) {
    return (
      <div className={`h-10 ${className ?? ""}`} aria-hidden>
        <div className="h-full w-full rounded bg-muted/40" />
      </div>
    )
  }

  const min = Math.min(...values)
  const max = Math.max(...values)
  // A flat series would divide by zero; render it down the middle instead.
  const span = max - min || 1

  const points = values
    .map((v, i) => {
      const x = (i / (values.length - 1)) * 100
      const y = 100 - ((v - min) / span) * 100
      return `${x.toFixed(2)},${y.toFixed(2)}`
    })
    .join(" ")

  return (
    <svg
      viewBox="0 0 100 100"
      preserveAspectRatio="none"
      className={`h-10 w-full ${className ?? ""}`}
      role="img"
      aria-label={`Trend, most recent value ${values[values.length - 1]}`}
    >
      <polyline
        points={points}
        fill="none"
        stroke="currentColor"
        strokeWidth={2}
        vectorEffect="non-scaling-stroke"
        strokeLinejoin="round"
        strokeLinecap="round"
      />
    </svg>
  )
}
