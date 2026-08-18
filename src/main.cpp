#include <Arduino.h>

#include "components/button/button.h"
#include "components/led/led.h"
#include "components/wifi/network_manager/network_manager.h"
#include "components/wifi/web_server/web_server.h"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  ledInit();
  buttonInit();

  Serial.println();
  Serial.println("esp32-iot-telemetry");

  // The button works with or without a network, so a failed join is not fatal.
  if (networkConnect()) {
    webServerBegin({ledOn, ledOff, ledToggle, ledIsOn, ledToggleCount});
  }
}

void loop() {
  // The ISR only counts; the work happens here, where neopixelWrite() and
  // Serial are safe to call.
  uint32_t presses = buttonTakePresses();
  for (uint32_t i = 0; i < presses; i++) {
    ledToggle();
    Serial.printf("button: led now %s (total %lu)\n", ledIsOn() ? "on" : "off",
                  ledToggleCount());
  }

  if (networkIsUp()) {
    webServerPoll();
  }
}
