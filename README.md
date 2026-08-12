# ESP32 Blinking LED

An LED blink built out as a V-Model exercise: each iteration runs a full requirements → design → implementation → test cycle, with regression tests carried forward.

| Version | Input  | Requirements                                     |
| ------- | ------ | ------------------------------------------------ |
| v1.0    | Timer  | REQ-1 blink at 200 ms; REQ-2 keep GPIO current in spec |
| v2.0    | Button | REQ-3 button press toggles the LED                |
| v3.0    | WiFi   | REQ-4 HTTP request toggles the LED                |

Full specification, traceability matrix and test plans are in [`docs/`](docs/).

## Hardware

ESP32-S3-WROOM-1 on a breadboard, USB powered.

| Pin    | Direction | Function        | Notes                          |
| ------ | --------- | --------------- | ------------------------------ |
| GPIO2  | Output    | On-board LED    | See caveat below               |
| GPIO4  | Output    | External LED    | 220 Ω series resistor          |
| GPIO13 | Input     | Push button     | `INPUT_PULLUP`, debounced 20–50 ms |

### Open issue: GPIO2 on the S3

The spec assigns the on-board LED to GPIO2, which is the ESP32-WROOM-32 convention. The ESP32-S3-DevKitC-1 has no plain LED on GPIO2 — it carries an addressable WS2812 RGB LED on GPIO38 (rev 1.0) or GPIO48 (rev 1.1) instead.

REQ-1's on-board LED output needs either a pin-map revision or an addressable-LED library before v1.0 can pass. GPIO4 and GPIO13 need no change.

## Building

PlatformIO is not installed on this machine. Install the [VS Code extension](https://platformio.org/install/ide?install=vscode) or the Core CLI first.

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial, 115200 baud
```

For v3.0, copy `include/secrets.h.example` to `include/secrets.h` and fill in your WiFi credentials. That file is gitignored.
