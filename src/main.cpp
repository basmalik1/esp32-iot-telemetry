#include <Arduino.h>

// Pin map per the v1.0 spec. See README for the GPIO2 caveat on ESP32-S3.
constexpr uint8_t PIN_LED_ONBOARD = 2;
constexpr uint8_t PIN_LED_EXTERNAL = 4;
constexpr uint8_t PIN_BUTTON = 13;

void setup() {
  Serial.begin(115200);
}

void loop() {
}
