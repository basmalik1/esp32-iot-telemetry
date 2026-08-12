# ESP32 Blinking LED

An LED blink built out as a V-Model exercise: each iteration runs a full requirements → design → implementation → test cycle, with regression tests carried forward.

| Version | Input  | Requirements                                     |
| ------- | ------ | ------------------------------------------------ |
| v1.0    | Timer  | REQ-1 blink at 200 ms; REQ-2 keep GPIO current in spec |
| v2.0    | Button | REQ-3 button press toggles the LED                |
| v3.0    | WiFi   | REQ-4 HTTP request toggles the LED                |

Full specification, traceability matrix and test plans are in [`docs/`](docs/).

## Hardware

Hosyond ESP32-S3 dev board (ESP32-S3-DevKitC-1 clone, dual Type-C) carrying an **ESP32-S3-WROOM-1-N16R8** module — 16 MB Quad SPI flash, 8 MB Octal SPI PSRAM. Breadboard, USB powered.

| Pin      | Direction | Function       | Notes                                  |
| -------- | --------- | -------------- | -------------------------------------- |
| GPIO48   | Output    | On-board RGB   | WS2812 addressable, not `digitalWrite` |
| GPIO4    | Output    | External LED   | 220 Ω series resistor                  |
| GPIO13   | Input     | Push button    | `INPUT_PULLUP`, debounced 20–50 ms     |

Because the module has Octal PSRAM (the `R8` suffix), **GPIO35, GPIO36 and GPIO37 are consumed by the PSRAM interface** and must not be used. GPIO4, GPIO13 and GPIO48 are all unaffected.

### LED Options

The spec assigns the on-board LED to GPIO2, which is the ESP32-WROOM-32 (classic DevKit v1) convention. It does not apply to this hardware.

**ESP32-S3-WROOM-1 is a module, not a dev board.** Its datasheet lists the only integrated components as a 40 MHz crystal and Quad SPI flash — there is no user LED on the module at all. Any on-board LED belongs to the carrier board the module is soldered to.

The carrier board has four LEDs in a row. Only the first is user-controllable:

| LED       | Pin    | Controllable |
| --------- | ------ | ------------ |
| WS2812 RGB | GPIO48 | Yes          |
| Power (red) | —     | No, hardwired |
| TX (green) | —      | No, driven by the USB-UART bridge |
| RX (blue)  | —      | No, driven by the USB-UART bridge |

Espressif's own DevKitC-1 uses GPIO48 on the initial revision and GPIO38 on v1.1; this clone follows the GPIO48 layout.

Being addressable, the RGB LED needs a WS2812 driver — Arduino-ESP32's built-in `neopixelWrite(RGB_BUILTIN, r, g, b)`, or `Adafruit_NeoPixel` / `FastLED` — not `digitalWrite()`.

**To verify on first flash:** some clones ship with the RGB LED's solder jumper open. If GPIO48 does nothing, inspect the pad near the LED before assuming a software fault.

REQ-1's on-board LED output therefore needs a decision before v1.0 can pass: either retarget it to GPIO48 and accept the WS2812 dependency, or drop the on-board LED from REQ-1 and verify against the external LED on GPIO4 alone.

Sources: [ESP32-S3-DevKitC-1 v1.1 user guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html), [ESP32-S3-WROOM-1 datasheet](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf), [YD-ESP32-S3 board reference](https://github.com/profharris/YD-ESP32-S3_ESP32-S3-WROOM-1_Dev)

## Building

PlatformIO is not installed on this machine. Install the [VS Code extension](https://platformio.org/install/ide?install=vscode) or the Core CLI first.

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial, 115200 baud
```

For v3.0, copy `include/secrets.h.example` to `include/secrets.h` and fill in your WiFi credentials. That file is gitignored.
