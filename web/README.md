# web

The telemetry dashboard the board serves at `/`. Vite + React + Tailwind + shadcn/ui.

```sh
npm install
npm run build                                    # -> dist/index.html (one file)
python ../tools/embed_web.py                     # -> ../src/.../web_ui.cpp
```

For live development against real hardware:

```sh
VITE_BOARD_URL=http://<board-ip> npm run dev
```

Full explanation, including why the bundle is the size it is, in
[docs/web-ui.md](../docs/web-ui.md).
