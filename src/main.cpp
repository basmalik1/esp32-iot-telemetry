#include <Arduino.h>

// Pin map per the v1.0 spec.
// The spec's on-board LED on GPIO2 does not exist on this board; the DevKitC-1
// carries an addressable RGB LED on GPIO48 (v1.0) or GPIO38 (v1.1). See README.
constexpr uint8_t PIN_LED_EXTERNAL = 4;
constexpr uint8_t PIN_BUTTON = 13;

void setup() {
  Serial.begin(115200);
}

void loop() {
}
