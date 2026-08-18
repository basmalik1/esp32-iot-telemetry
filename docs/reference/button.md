# button

Interrupt-driven, debounced push button on GPIO13.

```cpp
#include "components/button/button.h"

void     buttonInit();          // call once from setup()
uint32_t buttonTakePresses();   // presses since last call, then clears
```

## Behaviour

`buttonInit()` sets GPIO13 to `INPUT_PULLUP` and attaches an interrupt on the falling edge. With the pull-up the pin idles high, so a press to ground is a falling edge.

`buttonTakePresses()` returns how many presses have arrived since the previous call and resets the count. It returns a count rather than a boolean so that **two rapid presses produce two toggles** — a flag can only remember "at least one", which is the dropped-input failure TC-3.3 tests for.

Call it every `loop()` iteration:

```cpp
uint32_t presses = buttonTakePresses();
for (uint32_t i = 0; i < presses; i++) ledToggle();
```

## Debounce

Mechanical contacts bounce for roughly 1–5 ms, so one press produces several edges. The filter is **edge-plus-lockout**: accept the first edge immediately, then ignore further edges for `DEBOUNCE_MS` (50 ms). This responds on the leading edge rather than waiting for the signal to settle, so a press registers instantly.

The elapsed-time check is written as `now - lastIsrMs < DEBOUNCE_MS`. Unsigned subtraction wraps correctly, so this survives the `millis()` rollover at ~49.7 days; the more natural-looking `now < lastIsrMs + DEBOUNCE_MS` does not.

## Interrupt safety

The handler is marked `IRAM_ATTR`, placing it in RAM. An interrupt can fire while the SPI flash bus is busy, and an ISR that must be fetched from an unavailable bus crashes the chip.

It deliberately does no real work — `neopixelWrite()` drives the RMT peripheral and `Serial` performs I/O, and neither is safe in interrupt context. The handler debounces, increments a counter under a spinlock, and returns.

The counter is shared between the ISR and `loop()`. `volatile` prevents the compiler caching it, but it does not make read-modify-write atomic, so both sides take a `portMUX` spinlock. In `buttonTakePresses()` the read and the clear happen inside one critical section — as two separate statements, a press arriving between them would be silently overwritten.
