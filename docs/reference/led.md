# led

Drives the external LED on GPIO4 and the on-board WS2812 on GPIO48 as a single logical output. Both follow the same on/off state.

```cpp
#include "components/led/led.h"

void     ledInit();          // call once from setup()
void     ledOn();
void     ledOff();
void     ledToggle();
bool     ledIsOn();
uint32_t ledToggleCount();   // total toggles from every source
```

## Behaviour

`ledInit()` configures GPIO4 as an output and clears the LED. The clear is deliberate: a WS2812 latches its colour through a reset, so without it the board inherits whatever the previous firmware left lit.

GPIO48 needs no `pinMode` — `neopixelWrite()` routes the pin through the RMT peripheral itself.

The on colour is red at a per-channel value of **32** out of 255. Full scale is uncomfortable to look at and draws more current for no extra visibility.

`ledToggleCount()` increments on every `ledToggle()` regardless of what triggered it. It exists so a caller can prove no input was dropped: button presses plus HTTP toggles must equal the change in this number. See [TC-3.3](../process.md#tc-33-result).

## Notes

State lives in an anonymous namespace, so nothing outside `led.cpp` can write it — callers go through the functions.

Every caller runs in `loop()` context, including the HTTP handlers, so no locking is needed. That would change the moment an interrupt handler called `ledToggle()` directly, which is exactly why the button's ISR does not.
