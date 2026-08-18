#pragma once

#include <stdint.h>

// Drives both LEDs as one logical output: the external LED on GPIO4 and the
// on-board WS2812 on GPIO48.
void ledInit();
void ledOn();
void ledOff();
void ledToggle();

bool ledIsOn();

// Total toggles from every source. Lets a caller verify that no input was
// dropped under concurrent load (TC-3.3).
uint32_t ledToggleCount();
