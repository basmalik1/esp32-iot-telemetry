# Web UI

The board serves a React dashboard at `/` showing live telemetry — LED state, toggle count, signal strength, chip temperature, free heap and uptime — with controls for the LED. It polls `/status` once a second and keeps a minute of history for the sparklines. Light and dark themes, following the operating system until you choose otherwise.

Built with Vite, React, Tailwind and [shadcn/ui](https://ui.shadcn.com). Source lives in [`web/`](../web).

## How it reaches the board

There is no filesystem upload step. The built page is compiled into the firmware, so one `pio run -t upload` ships both.

```
web/src/**          →  npm run build   →  web/dist/index.html   (one file)
web/dist/index.html →  embed_web.py    →  src/.../web_ui.cpp    (gzipped bytes)
web_ui.cpp          →  pio run         →  firmware
```

Three decisions make that work:

**One file, not many.** `vite-plugin-singlefile` inlines the JS and CSS into `index.html`, so the firmware has exactly one asset to embed and one route to serve. No MIME-type table, no asset paths, no second request.

**Pre-gzipped at build time.** The board never compresses anything at runtime — it stores the gzipped bytes and sets `Content-Encoding: gzip`. 266 kB becomes 80 kB, served in about 120 ms.

**Generated source is committed.** `web_ui.cpp` is in the repo, so cloning and flashing needs no Node at all. Only *changing* the UI does.

## Changing the UI

```sh
cd web
npm install          # first time only
npm run build
cd ..
python tools/embed_web.py
pio run -t upload
```

Forget `embed_web.py` and you will flash the previous dashboard while wondering why your change did nothing.

### Working on it live

Rebuilding and reflashing for every CSS tweak is miserable. Point the dev server at a real board instead:

```sh
cd web
VITE_BOARD_URL=http://<board-ip> npm run dev
```

The page then runs on `localhost:5173` with hot reload while talking to real hardware. In production the variable is unset and all requests are relative, which is why the same code works both ways.

## Size, and why it matters

This lives in flash and is pushed through a single-threaded server, so bytes are not free.

| | gzipped |
| --- | --- |
| Initial build (shadcn defaults) | 158 kB |
| After dropping the bundled font | **80 kB** |

shadcn's default style inlines the Geist variable font. Swapping it for the system font stack halved the bundle for no visible loss — the dashboard looks native on whatever opens it.

For the same reason the sparklines are a hand-rolled SVG polyline rather than a charting library: Recharts would have added roughly another 100 kB for a line with no axes.

## Constraints worth knowing

**Assets are stored gzipped only.** There is no uncompressed copy, so `Content-Encoding: gzip` is sent unconditionally. Every browser sends `Accept-Encoding: gzip`, so this is invisible in practice — but a bare HTTP client that does not decompress will receive binary. `tools/system_test.py` handles this explicitly.

**Polling is serialised.** The dashboard skips a poll if one is still in flight. The board answers one request at a time, so overlapping polls would queue behind each other and make the UI feel worse rather than better.

**Theme is applied before React mounts.** A small inline script in `index.html` reads the stored preference — or `prefers-color-scheme` if there is none — and sets the class on `<html>` during parse. Waiting for React would mean a white flash on every load for dark-mode users, on a page that takes ~120 ms to arrive. The React hook reads the same key afterwards, so the two never disagree.

**The physical button and the dashboard share state.** Press the button and the toggle count on screen moves within a second — the same counter that [TC-3.3](process.md#tc-33-result) uses to prove no input is ever dropped.
