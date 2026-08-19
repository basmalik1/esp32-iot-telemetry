# ESP32 IoT Telemetry

An ESP32-S3 that measures itself and reports die temperature, signal strength, and free heap over WiFi. It also serves its own dashboard to display it. It can take commands back from the browser or from a physical button on the breadboard which drive LEDs.

The neat part is that **the board is the web server** — no cloud, broker, or companion app. The 80 kB of dashboard is compiled into the firmware and served from flash. Also, **a hardware interrupt and that web server share one piece of state without ever losing an input**, which is validated through a test that drives both at once for twenty seconds and checks the arithmetic.

![Breadboard](docs/img/breadboard-v2.png)

## Telemetry

`GET /status` returns everything the device knows about itself:

```console
$ curl http://<board-ip>/status
{"led":false,"toggles":42,"rssi":-31,"uptime_ms":65138,"free_heap":292260,"temp_c":45.8}
```

| Field | What it is |
| --- | --- |
| `temp_c` | ESP32-S3 internal die sensor — the chip's temperature, not the room's |
| `rssi` | WiFi signal strength in dBm |
| `free_heap` | Heap bytes remaining, which is how you spot a leak over a long run |
| `uptime_ms` | Milliseconds since boot |
| `toggles` | Every LED change from either input — the counter the concurrency test relies on |
| `led` | Current output state |

The dashboard polls this once a second and keeps a minute of history for the sparklines.

## Control

| Input | How it works |
| --- | --- |
| Button | GPIO interrupt with a 50 ms debounce, counted so rapid presses are never collapsed |
| HTTP | `GET /on`, `/off`, `/toggle` on port 80 |

Every endpoint replies with the full status object, so a caller never needs a second request to find out what happened.

## Dashboard

Browse to the board's IP to see live cards, sparklines for temperature and signal, LED controls, and light/dark themes that follow your system until you pick one. Press the physical button and the count on screen moves within a second.

The whole page is compiled into the firmware as pre-gzipped bytes and served straight from flash in about 120 ms — no filesystem upload step, and no Node toolchain needed to flash it. [docs/web-ui.md](docs/web-ui.md) covers how it is built and why it is that size.

## Hardware

Hosyond ESP32-S3 dev board — a DevKitC-1 clone carrying an ESP32-S3-WROOM-1-N16R8 module. Breadboard, USB powered.

| Pin | Direction | Connected to |
| --- | --- | --- |
| GPIO4 | Output | External LED, through a 220 Ω series resistor |
| GPIO13 | Input | Push button to ground, using the internal pull-up |
| GPIO48 | Output | On-board WS2812 RGB LED |

Board quirks worth knowing before you wire anything — the unusable PSRAM pins, which USB port carries `Serial`, and the flash-size setting that boot-loops the board if you get it wrong — are in [docs/hardware.md](docs/hardware.md).

## Getting started

Copy `include/secrets.h.example` to `include/secrets.h` and fill in your WiFi credentials; it is gitignored — **note: the ESP32-S3 only sees 2.4 GHz networks.**

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial, 115200 baud
```

If `pio` is not on your `PATH`, run these from the PlatformIO terminal in VS Code. The board's IP is printed on boot.

## Tests

```sh
pio test -e native   # 14 tests, no hardware needed
pio test -e target   # 9 tests, on the board
python tools/system_test.py --host <board-ip>
```

Three tiers, split by what each can prove — see [docs/testing.md](docs/testing.md). The host tier is what makes the `millis()` rollover testable at all; reaching it on real hardware would take 49.7 days of uptime.

## Structure

```
src/components/
  led/
  button/
  wifi/
    network_manager/
    web_server/
```

Components never reach into each other — `main.cpp` is the only file that knows about more than one. The web server is handed a struct of function pointers rather than including the LED module, so it depends on no concrete output and can be tested against a fake. [docs/architecture.md](docs/architecture.md) covers the reasoning, including the interrupt-vs-loop concurrency.

## Documentation

[docs/](docs/) — [hardware](docs/hardware.md) · [architecture](docs/architecture.md) · [process](docs/process.md) · [testing](docs/testing.md) · [module reference](docs/reference/)

Built as a V-Model exercise: every requirement is traced to the code that satisfies it and the test that proves it, with every test case in the plan executed. That trail is in [docs/process.md](docs/process.md).

## Release Notes

The version history and artifacts associated with the project can be found at [Releases](https://github.com/basmalik1/esp32-iot-telemetry/releases).
