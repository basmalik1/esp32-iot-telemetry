# network_manager

Joins the WiFi network and reports link state. Owns nothing else — serving HTTP is [web_server](web-server.md)'s job.

```cpp
#include "components/wifi/network_manager/network_manager.h"

bool      networkConnect();   // returns false instead of blocking forever
bool      networkIsUp();
IPAddress networkIp();
int       networkRssi();
```

## Credentials

Read from `include/secrets.h`, which is gitignored. Copy `include/secrets.h.example` and fill it in:

```c
#define WIFI_SSID     "your-network-name"
#define WIFI_PASSWORD "your-password"
```

Without that file the build fails at the `#include`. That is intentional — a missing configuration should be a compile error, not a board that silently never joins.

## Behaviour

`WiFi.begin()` returns immediately; the association completes in the background. `networkConnect()` polls `WiFi.status()` until it lands or 30 seconds elapse, then returns success or failure rather than deciding what a failure means. `main.cpp` treats it as non-fatal — the button works with no network.

On success the hostname is set to `esp32-iot-telemetry`, so the board is identifiable in a router's client list instead of appearing as a bare MAC address.

WiFi modem sleep is disabled after connecting. Sleep parks the radio between router beacons, which showed up as 80–300 ms round trips; disabling it costs roughly 20–30 mA and brings HTTP responses down to around 13 ms. That is the right trade for a USB-powered board, and the wrong one for a battery.

## Failure diagnostics

A failed join logs every network the radio can see, with channel and signal strength. This exists because "SSID not found" and "password rejected" are indistinguishable from the outside — the scan turns a guess into data.

The status code is worth reading before changing anything:

| Code | Meaning | Usual cause |
| --- | --- | --- |
| 1 | `WL_NO_SSID_AVAIL` | Network never seen — wrong name, or 5 GHz |
| 4 | `WL_CONNECT_FAILED` | Credentials rejected |
| 6 | `WL_DISCONNECTED` | Association failed or dropped |

**The ESP32-S3 radio is 2.4 GHz only.** If a router publishes its bands under separate names, the 5 GHz SSID is simply invisible to the board and reports status 1. A longer timeout will not help.

## Known gap

There is no reconnect. If the access point restarts, `networkIsUp()` keeps returning true while the server is unreachable, until the board is reset. Fixing it means re-checking `WiFi.status()` periodically in `loop()`.
