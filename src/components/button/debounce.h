#pragma once

#include <stdint.h>

// The debounce decision, kept free of any Arduino dependency so it can be
// unit-tested on the host. It is `inline` deliberately: the interrupt handler
// calls it, and inlining places it inside the handler's IRAM rather than
// needing IRAM_ATTR of its own.
//
// Returns true if an edge arriving at `now` is far enough from the last
// accepted edge to be a real press rather than contact bounce.
//
// The unsigned subtraction is load-bearing. millis() wraps to zero after about
// 49.7 days; `now - lastAccepted` stays correct across that boundary, whereas
// the more natural-looking `now < lastAccepted + debounceMs` overflows and
// stops accepting presses.
inline bool buttonDebounceAccepts(uint32_t now, uint32_t lastAccepted,
                                  uint32_t debounceMs) {
  return (now - lastAccepted) >= debounceMs;
}
