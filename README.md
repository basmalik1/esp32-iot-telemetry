# ESP32 IoT Telemetry

An ESP32-S3 that drives an LED two different ways: from a physical button, or over HTTP from any device on the same network.

The LED is not the interesting part. The interesting part is that a hardware interrupt and a web server share one piece of state without ever losing an input — and there is a test that proves it, by driving both at once for twenty seconds and checking the arithmetic.

![Breadboard](docs/img/breadboard-v2.png)

## Inputs

| Input | How it works |
| --- | --- |
| Button | GPIO interrupt with a 50 ms debounce, counted so rapid presses are never collapsed |
| HTTP | `GET /on`, `/off`, `/toggle`, `/status` on port 80 |

Every endpoint replies with the resulting state, so a caller never needs a second request:

```console
$ curl http://<board-ip>/toggle
{"led":true,"toggles":42,"rssi":-31,"uptime_ms":65138}
```

There is also a plain page of links at `/`, so the board can be driven from a phone browser with no tooling.

## Hardware

Hosyond ESP32-S3 dev board — a DevKitC-1 clone carrying an ESP32-S3-WROOM-1-N16R8 module. Breadboard, USB powered.

| Pin | Direction | Connected to |
| --- | --- | --- |
| GPIO4 | Output | External LED, through a 220 Ω series resistor |
| GPIO13 | Input | Push button to ground, using the internal pull-up |
| GPIO48 | Output | On-board WS2812 RGB LED |

Board quirks worth knowing before you wire anything — the unusable PSRAM pins, which USB port carries `Serial`, and the flash-size setting that boot-loops the board if you get it wrong — are in [docs/hardware.md](docs/hardware.md).

## Getting started

Copy `include/secrets.h.example` to `include/secrets.h` and fill in your WiFi credentials; it is gitignored — **note:** the ESP32-S3 only sees 2.4 GHz networks.

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial, 115200 baud
```

If `pio` is not on your `PATH`, run these from the PlatformIO terminal in VS Code. The board's IP is printed on boot.

## Tests

```sh
pio test -e native   # 13 tests, no hardware needed
pio test -e target   # 9 tests, on the board
python tools/system_test.py --host <board-ip>
```

Three tiers, split by what each can prove — see [docs/testing.md](docs/testing.md). The host tier is what makes the `millis()` rollover testable at all; reaching it on real hardware would take 49.7 days of uptime.

## Structure

```
src/components/
  led/        button/
  wifi/network_manager/  wifi/web_server/
```

Components never reach into each other — `main.cpp` is the only file that knows about more than one. The web server is handed a struct of function pointers rather than including the LED module, so it depends on no concrete output and can be tested against a fake. [docs/architecture.md](docs/architecture.md) covers the reasoning, including the interrupt-vs-loop concurrency.

## Documentation

[docs/](docs/) — [hardware](docs/hardware.md) · [architecture](docs/architecture.md) · [process](docs/process.md) · [testing](docs/testing.md) · [module reference](docs/reference/)

Built as a V-Model exercise: every requirement is traced to the code that satisfies it and the test that proves it, including the cases that are still unproven. That trail is in [docs/process.md](docs/process.md).

## Release Notes

The version history and artifacts associated with the project can be found at [Releases](https://github.com/basmalik1/esp32-iot-telemetry/releases).
