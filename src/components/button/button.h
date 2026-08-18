#pragma once

#include <stdint.h>

// Interrupt-driven, debounced push button on GPIO13.
void buttonInit();

// Returns how many presses have arrived since the last call, and clears the
// count. Presses are never collapsed: two rapid presses return 2, not 1.
uint32_t buttonTakePresses();
