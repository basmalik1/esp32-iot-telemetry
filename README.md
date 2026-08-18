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
| GPIO4    | Output    | External LED   | 220 Ω series resistor                  |
| GPIO13   | Input     | Push button    | `INPUT_PULLUP`, debounced 20–50 ms     |
| GPIO48   | Output    | On-board RGB   | WS2812 addressable, not `digitalWrite` |

Because the module has Octal PSRAM (the `R8` suffix), **GPIO35, GPIO36 and GPIO37 are consumed by the PSRAM interface** and must not be used. GPIO4, GPIO13 and GPIO48 are all unaffected.

### LED Options

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

Sources: [ESP32-S3-DevKitC-1 v1.1 user guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html), [ESP32-S3-WROOM-1 datasheet](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf), [YD-ESP32-S3 board reference](https://github.com/profharris/YD-ESP32-S3_ESP32-S3-WROOM-1_Dev)

## Configuration

v3.0 connects to WiFi, and the credentials are deliberately not in this repo. **If you cloned this, you must create `include/secrets.h` before the project will build.**

1. Copy the committed template:

   ```sh
   cp include/secrets.h.example include/secrets.h
   ```

2. Edit `include/secrets.h` with your own network:

   ```c
   #define WIFI_SSID     "your-network-name"
   #define WIFI_PASSWORD "your-password"
   ```

`include/secrets.h` is gitignored and must never be committed; `include/secrets.h.example` is the template that is. Leaving `secrets.h` out breaks the build at `#include "secrets.h"` — that is intentional, so a missing configuration surfaces as a compile error rather than a board that silently fails to join the network.

**The ESP32-S3 has a 2.4 GHz radio only.** If your router publishes 2.4 GHz and 5 GHz under separate names, use the 2.4 GHz SSID. SSIDs are case-sensitive.

## Building

PlatformIO Core 6.1.19 is installed at `~/.platformio/penv/Scripts/pio.exe` but is not on `PATH`, so a bare `pio` fails from an ordinary shell. Use the full path, or run these from VS Code's PlatformIO terminal where `pio` resolves.

```sh
~/.platformio/penv/Scripts/pio.exe run              # build
~/.platformio/penv/Scripts/pio.exe run -t upload    # flash
~/.platformio/penv/Scripts/pio.exe device monitor   # serial, 115200 baud
```

The board has two USB-C ports and `Serial` comes out the **UART** one, not the native USB port — see [docs/hardware.md](docs/hardware.md) if nothing appears in the monitor.
