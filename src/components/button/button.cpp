#include "button.h"

#include <Arduino.h>

namespace {

constexpr uint8_t PIN_BUTTON = 13;
constexpr uint32_t DEBOUNCE_MS = 50;

// Shared with the ISR. volatile forces every access to memory - otherwise the
// compiler may cache these in registers, since nothing it can see modifies
// them, and the ISR's writes would never be observed.
volatile uint32_t pendingPresses = 0;
volatile uint32_t lastIsrMs = 0;

// volatile is not atomicity. It stops caching, but a read-modify-write can
// still be interrupted halfway. This spinlock makes those sequences
// indivisible, and on ESP32 also guards against the other core.
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// IRAM_ATTR keeps this in RAM. An interrupt can fire while the SPI flash bus
// is busy, and an ISR that must be fetched from an unavailable bus crashes the
// chip.
//
// It deliberately does no real work: neopixelWrite() and Serial are both
// unsafe here. Counting and returning keeps it to a handful of instructions.
void IRAM_ATTR onPress() {
  uint32_t now = millis();
  if (now - lastIsrMs < DEBOUNCE_MS) {
    return; // contact bounce
  }
  lastIsrMs = now; // only the ISR touches this, so it needs no lock

  portENTER_CRITICAL_ISR(&mux);
  pendingPresses++;
  portEXIT_CRITICAL_ISR(&mux);
}

} // namespace

void buttonInit() {
  // With a pull-up the pin idles HIGH, so a press is a falling edge.
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), onPress, FALLING);
}

uint32_t buttonTakePresses() {
  uint32_t count;
  // Read and clear must be one operation. As two statements, a press arriving
  // in between is silently overwritten and lost.
  portENTER_CRITICAL(&mux);
  count = pendingPresses;
  pendingPresses = 0;
  portEXIT_CRITICAL(&mux);
  return count;
}
