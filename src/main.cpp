#include <Arduino.h>

// Pin map per the v1.0 spec.
// The spec's on-board LED on GPIO2 does not exist on this board. The on-board
// LED is a WS2812 on GPIO48 and needs neopixelWrite(), not digitalWrite().
constexpr uint8_t PIN_LED_ONBOARD_RGB = 48;
constexpr uint8_t PIN_LED_EXTERNAL = 4;
constexpr uint8_t PIN_BUTTON = 13;

void setup() {
  Serial.begin(115200);
}

void loop() {
}
