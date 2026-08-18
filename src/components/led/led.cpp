#include "led.h"

#include <Arduino.h>

namespace {

constexpr uint8_t PIN_EXTERNAL = 4;     // through a 220 ohm series resistor
constexpr uint8_t PIN_ONBOARD_RGB = 48; // WS2812, needs neopixelWrite()

// Per channel, out of 255. Full scale is hard to look at and draws more
// current for no extra visibility.
constexpr uint8_t ONBOARD_LEVEL = 32;

bool state = false;
uint32_t toggles = 0;

} // namespace

void ledInit() {
  pinMode(PIN_EXTERNAL, OUTPUT);
  // The WS2812 needs no pinMode - neopixelWrite() routes the pin through the
  // RMT peripheral itself. It also latches its colour through a reset, so
  // clear it rather than inheriting whatever was left on.
  ledOff();
}

void ledOn() {
  neopixelWrite(PIN_ONBOARD_RGB, ONBOARD_LEVEL, 0, 0);
  digitalWrite(PIN_EXTERNAL, HIGH);
  state = true;
}

void ledOff() {
  neopixelWrite(PIN_ONBOARD_RGB, 0, 0, 0);
  digitalWrite(PIN_EXTERNAL, LOW);
  state = false;
}

void ledToggle() {
  if (state) {
    ledOff();
  } else {
    ledOn();
  }
  toggles++;
}

bool ledIsOn() { return state; }

uint32_t ledToggleCount() { return toggles; }
