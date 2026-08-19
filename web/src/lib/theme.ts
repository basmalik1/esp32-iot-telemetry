import { useEffect, useState } from "react"

export type Theme = "light" | "dark"

const STORAGE_KEY = "theme"

// An explicit choice wins; otherwise follow the operating system. Reading this
// lazily in useState means the first render already has the right value, so
// nothing repaints on mount.
function preferredTheme(): Theme {
  try {
    const stored = localStorage.getItem(STORAGE_KEY)
    if (stored === "light" || stored === "dark") return stored
  } catch {
    // Some browsers throw on localStorage in private mode. Fall through to the
    // system preference rather than failing to render.
  }
  return window.matchMedia?.("(prefers-color-scheme: dark)").matches
    ? "dark"
    : "light"
}

export function useTheme() {
  const [theme, setTheme] = useState<Theme>(preferredTheme)

  useEffect(() => {
    const root = document.documentElement
    // Tailwind's dark variant is configured as `&:is(.dark *)`, so the class
    // goes on <html> and the tokens cascade to everything.
    root.classList.toggle("dark", theme === "dark")
    // Keeps native scrollbars and form controls in step with the page.
    root.style.colorScheme = theme
    try {
      localStorage.setItem(STORAGE_KEY, theme)
    } catch {
      // Persisting is a convenience; the UI still works without it.
    }
  }, [theme])

  return {
    theme,
    toggle: () => setTheme((t) => (t === "dark" ? "light" : "dark")),
  }
}
