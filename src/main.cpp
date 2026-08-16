#include <Arduino.h>

// Define Pins
constexpr uint8_t PIN_LED_EXTERNAL = 4; // GPIO4 drives the external LED through its 220 ohm series resistor.
constexpr uint8_t PIN_ONBOARD_RGB = 48; // GPIO48 drives the on-board addressable RGB LED.
constexpr uint8_t PIN_BUTTON = 13; // GPIO13 is the button input.

// Define LED Brightness
constexpr uint8_t ONBOARD_LEVEL = 32; // The per-channel value used for "on" below: 32 out of a possible 255, i.e. about 12% duty. Raising this makes the LED brighter and hungrier.

// Debounce interval
constexpr uint8_t DEBOUNCE_MS = 20; // Keep around 20-50ms

// Human readable color structure
struct Color {
  const char *name;
  uint8_t r, g, b;
};

// RED output for LED
constexpr Color RED = {"red", ONBOARD_LEVEL, 0, 0};

// LED status variable
bool ledStatus = false;

void ledOn() {
  neopixelWrite(PIN_ONBOARD_RGB, RED.r, RED.g, RED.b);
  digitalWrite(PIN_LED_EXTERNAL, HIGH);
  ledStatus = true;
}

void ledOff() {
  neopixelWrite(PIN_ONBOARD_RGB, 0, 0, 0);
  digitalWrite(PIN_LED_EXTERNAL, LOW);
  ledStatus = false;
}

void toggleLed() {
  if (ledStatus) {
    ledOff();
  } else {
    ledOn();
  }
}

bool readButton() {
  static bool lastStable = HIGH; // The last stable state of the button (HIGH means not pressed, LOW means pressed)
  static uint32_t lastChange = 0; // The last time the button state changed

  bool now = digitalRead(PIN_BUTTON); // Current raw input from the button
  if ((now != lastStable) && (millis() - lastChange >= DEBOUNCE_MS)) { // If the button state has changed and the debounce interval has passed
    lastChange = millis(); // Update the last change time
    lastStable = now; // Update the last stable state
    return (now == LOW); // fire on press, not release
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  pinMode(PIN_LED_EXTERNAL, OUTPUT);
  pinMode(PIN_ONBOARD_RGB, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  Serial.println();
  Serial.println("Hello, world!");
}

void loop() {
  if (readButton()) {
    toggleLed();
  }
}