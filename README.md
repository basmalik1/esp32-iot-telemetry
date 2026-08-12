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

| Pin      | Direction | Function     | Notes                              |
| -------- | --------- | ------------ | ---------------------------------- |
| ~~GPIO2~~ | Output   | On-board LED | **Invalid on this board — see below** |
| GPIO4    | Output    | External LED | 220 Ω series resistor              |
| GPIO13   | Input     | Push button  | `INPUT_PULLUP`, debounced 20–50 ms |

### Open issue: there is no on-board LED on GPIO2

The spec assigns the on-board LED to GPIO2, which is the ESP32-WROOM-32 (classic DevKit v1) convention. It does not apply to this hardware.

**ESP32-S3-WROOM-1 is a module, not a dev board.** Its datasheet lists the only integrated components as a 40 MHz crystal and Quad SPI flash — there is no user LED on the module at all. Any on-board LED belongs to the carrier board the module is soldered to.

On the ESP32-S3-DevKitC-1 carrier, the only user-controllable LED is an **addressable RGB LED**, and its pin depends on board revision:

| Board revision | RGB LED pin |
| -------------- | ----------- |
| Initial (v1.0) | GPIO48      |
| v1.1           | GPIO38      |

Being addressable, it needs a WS2812 driver (`Adafruit_NeoPixel`, `FastLED`, or Arduino-ESP32's `neopixelWrite()`), not `digitalWrite()`.

REQ-1's on-board LED output therefore needs a decision before v1.0 can pass. GPIO4 and GPIO13 are unaffected and need no change.

Sources: [ESP32-S3-DevKitC-1 v1.1 user guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html), [ESP32-S3-WROOM-1 datasheet](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)

## Building

PlatformIO is not installed on this machine. Install the [VS Code extension](https://platformio.org/install/ide?install=vscode) or the Core CLI first.

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial, 115200 baud
```

For v3.0, copy `include/secrets.h.example` to `include/secrets.h` and fill in your WiFi credentials. That file is gitignored.
