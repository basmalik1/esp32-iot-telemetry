# web_server

HTTP control surface on port 80. Knows nothing about LEDs — it drives whatever it is handed.

```cpp
#include "components/wifi/web_server/web_server.h"

struct LedControl {
  void (*on)();
  void (*off)();
  void (*toggle)();
  bool (*isOn)();
  uint32_t (*toggles)();
};

void webServerBegin(const LedControl &led);   // after the network is up
void webServerPoll();                          // every loop() iteration
```

## Endpoints

| Method | Path | Effect |
| --- | --- | --- |
| GET | `/` | Page of links, so the board can be driven from a phone browser |
| GET | `/on` | Turn on |
| GET | `/off` | Turn off |
| GET | `/toggle` | Toggle |
| GET | `/status` | Read current state |
| — | anything else | `404` |

Every endpoint — including the three that change state — replies with the resulting state:

```json
{"led":true,"toggles":42,"rssi":-31,"uptime_ms":65138}
```

So a caller never needs a second request to find out what happened.

```sh
curl http://<board-ip>/toggle
```

Actions are `GET` rather than `POST` so they can be triggered from a browser address bar with no tooling. That is a deliberate trade against REST convention, where `GET` is expected to be side-effect free — worth revisiting if anything ever prefetches these URLs.

## Why the injected struct

`webServerBegin()` takes function pointers instead of including `led.h`. The module therefore depends on no concrete output: it can be lifted into another project unchanged, and its endpoints can be unit-tested against a fake `LedControl` on a development machine with no hardware attached.

`main.cpp` does the wiring:

```cpp
webServerBegin({ledOn, ledOff, ledToggle, ledIsOn, ledToggleCount});
```

It does include [network_manager](network-manager.md), for `networkRssi()` and `networkIp()`. Both live inside the `wifi/` group, so that dependency is internal to the group rather than a component reaching across to a sibling.

## Timing

The server is **synchronous**. A request sits in the socket buffer, unanswered, until `webServerPoll()` runs. Any blocking call in `loop()` becomes HTTP latency one-for-one — a `delay(200)` would mean requests taking up to 200 ms longer.

Measured round trip on the local network is about 13 ms with modem sleep disabled.

Responses are built with `snprintf` into a fixed buffer rather than an Arduino `String`. Handlers run thousands of times, and repeated heap allocation in a long-running embedded process fragments memory; the size argument also makes overflow impossible.
