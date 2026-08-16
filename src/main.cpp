#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h" // WIFI_SSID and WIFI_PASSWORD. Gitignored, see README.

// Listens on the standard HTTP port, so URLs need no :port suffix.
WebServer server(80);

// Set once the join succeeds. loop() must not poll a server that never started.
bool wifiReady = false;

// Define Pins
constexpr uint8_t PIN_LED_EXTERNAL = 4; // GPIO4 drives the external LED through its 220 ohm series resistor.
constexpr uint8_t PIN_ONBOARD_RGB = 48; // GPIO48 drives the on-board addressable RGB LED.
constexpr uint8_t PIN_BUTTON = 13; // GPIO13 is the button input.

// Define LED Brightness
constexpr uint8_t ONBOARD_LEVEL = 32; // The per-channel value used for "on" below: 32 out of a possible 255, i.e. about 12% duty. Raising this makes the LED brighter and hungrier.

// Debounce interval
constexpr uint8_t DEBOUNCE_MS = 50; // Keep around 20-50ms

// How long to wait for the network before giving up. Without a bound the board
// would sit here forever if the router were down.
constexpr uint32_t WIFI_TIMEOUT_MS = 30000;

// Shows up in the router's client list instead of a generic espressif MAC.
constexpr char WIFI_HOSTNAME[] = "esp32-led";

// Human readable color structure
struct Color {
  const char *name;
  uint8_t r, g, b;
};

// RED output for LED
constexpr Color RED = {"red", ONBOARD_LEVEL, 0, 0};

// LED status variable
bool ledStatus = false;

// Counts every toggle from either input path, which is what makes TC-3.3
// measurable rather than a judgement call: button presses plus HTTP toggles
// must equal the change in this number. Only ever touched from loop() context
// - the ISR never calls toggleLed() - so it needs no lock.
uint32_t toggleCount = 0;

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
  toggleCount++;
}

// Written by the ISR, read by loop(). volatile forces every access to go to
// memory: without it the compiler may cache these in registers, since nothing
// it can see in loop() ever modifies them, and the ISR's writes would be
// invisible forever.
//
// A counter, not a bool: two presses arriving before loop() services them must
// produce two toggles. A flag can only remember "at least one", which is the
// "forgotten request" TC-3.3 tests for.
volatile uint32_t pendingPresses = 0;
volatile uint32_t lastIsrMs = 0; // when the last accepted press arrived

// volatile alone is not enough. It stops the compiler caching a value, but
// pendingPresses++ and the read-then-clear in loop() are read-modify-write
// sequences that an interrupt can land in the middle of. This spinlock makes
// those sequences indivisible - and on ESP32 it also guards against the other
// core, since the WiFi stack really does run on core 0 while loop() runs on 1.
portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;

// The spec's handleButton(). Replaces the polled readButton() from v2.0, which
// remains in history at the v2.0 tag.
//
// IRAM_ATTR places this function in RAM rather than flash. An interrupt can
// fire while the SPI flash bus is busy, and an ISR that must be fetched from
// an unavailable bus crashes the chip.
//
// It does no real work: neopixelWrite() drives the RMT peripheral and is not
// interrupt-safe, and Serial is not either. Setting a flag and returning keeps
// this a handful of instructions, and loop() does everything that matters.
void IRAM_ATTR handleButton() {
  uint32_t now = millis();
  if (now - lastIsrMs < DEBOUNCE_MS) {
    return; // contact bounce - same lockout rule readButton() used
  }
  lastIsrMs = now; // only this ISR touches it, so it needs no lock

  // The _ISR variants are the ones to use from interrupt context. Keep the
  // section to a single increment: it disables interrupts while held.
  portENTER_CRITICAL_ISR(&buttonMux);
  pendingPresses++;
  portEXIT_CRITICAL_ISR(&buttonMux);
}

// Reads the counter and zeroes it as one indivisible operation, returning how
// many presses were waiting. Doing this as two separate statements is the bug:
// an interrupt landing between the read and the clear loses that press.
uint32_t takePendingPresses() {
  uint32_t count;
  portENTER_CRITICAL(&buttonMux);
  count = pendingPresses;
  pendingPresses = 0;
  portEXIT_CRITICAL(&buttonMux);
  return count;
}

// Lists every network the radio can actually see. Called when a join fails,
// because from the outside "network not found" and "password rejected" look
// identical - this turns the guess into data. The ESP32-S3 radio is 2.4 GHz
// only, so anything on 5 GHz will simply be absent from this list.
void wifiScan() {
  Serial.println("wifi: scanning...");
  WiFi.disconnect(); // drop the failed association so the scan runs clean
  delay(100);

  int found = WiFi.scanNetworks();
  if (found <= 0) {
    Serial.println("wifi: no networks visible at all");
    return;
  }

  Serial.printf("wifi: %d networks visible\n", found);
  for (int i = 0; i < found; i++) {
    // SSID(i) returns an Arduino String, so .c_str() is needed for printf.
    Serial.printf("  %-32s ch%-3d %4d dBm %s\n", WiFi.SSID(i).c_str(),
                  WiFi.channel(i), WiFi.RSSI(i),
                  WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "secured");
  }
  WiFi.scanDelete(); // free the scan results
}

// Joins the network. Returns true on success so the caller can decide what a
// failure means, rather than this function deciding for it.
bool wifiSetup() {
  WiFi.mode(WIFI_STA);              // station (client), not access point
  WiFi.setHostname(WIFI_HOSTNAME);  // must be set before begin()
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.printf("wifi: connecting to %s", WIFI_SSID);

  // begin() returns immediately - the association happens in the background,
  // so poll status() until it lands or the timeout expires.
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("wifi: FAILED after %lu ms, status %d\n", millis() - start,
                  WiFi.status());
    wifiScan(); // show what was actually in range
    return false;
  }

  // localIP() returns an IPAddress object, not a string, so it needs println's
  // overload - it cannot be passed to printf's %s.
  Serial.print("wifi: connected, IP ");
  Serial.println(WiFi.localIP());
  Serial.printf("wifi: rssi %d dBm\n", WiFi.RSSI());

  // Modem sleep parks the radio between router beacons, which showed up as
  // 80-300 ms ping times. Disabling it costs roughly 20-30 mA but makes HTTP
  // responses feel immediate - the right trade for a USB-powered board.
  WiFi.setSleep(false);
  return true;
}

// Every endpoint answers with the resulting state, so a caller never has to
// make a second request to find out what happened.
void sendStatus() {
  // snprintf into a fixed buffer rather than building an Arduino String: no
  // heap allocation, and the size argument makes overflow impossible.
  char body[160];
  snprintf(body, sizeof(body),
           "{\"led\":%s,\"toggles\":%lu,\"rssi\":%d,\"uptime_ms\":%lu}\n",
           ledStatus ? "true" : "false", toggleCount, WiFi.RSSI(), millis());
  server.send(200, "application/json", body);
}

// A plain page of links, so the board can be driven from a phone browser on
// the same network without any tooling.
void handleRoot() {
  static const char page[] =
      "<!doctype html><meta name=viewport content=\"width=device-width\">"
      "<h2>ESP32 LED</h2><p>"
      "<a href=\"/on\">on</a> | <a href=\"/off\">off</a> | "
      "<a href=\"/toggle\">toggle</a> | <a href=\"/status\">status</a></p>";
  server.send(200, "text/html", page);
}

// The three actions reuse the exact functions the button already calls, so
// both input paths converge on one implementation rather than two.
void handleOn() {
  ledOn();
  sendStatus();
}

void handleOff() {
  ledOff();
  sendStatus();
}

void handleToggle() {
  toggleLed();
  sendStatus();
}

void handleNotFound() {
  server.send(404, "text/plain", "no such endpoint\n");
}

// Registers the routes and opens the socket. Split from wifiSetup() because
// joining a network and serving HTTP are separate concerns.
void webSetup() {
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/toggle", handleToggle);
  server.on("/status", sendStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.print("http: serving at http://");
  Serial.println(WiFi.localIP());
}

// The spec's handleWifi(): called every loop() iteration to service any
// pending request. WebServer is synchronous, so nothing happens unless this
// runs - which is exactly why loop() must never block.
void handleWifi() {
  server.handleClient();
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  pinMode(PIN_LED_EXTERNAL, OUTPUT);
  pinMode(PIN_ONBOARD_RGB, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // With INPUT_PULLUP the pin idles HIGH, so a press is a falling edge.
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), handleButton, FALLING);

  Serial.println();
  Serial.println("Hello, world!");

  // The button keeps working regardless, so a failed join is not fatal here.
  wifiReady = wifiSetup();
  if (wifiReady) {
    webSetup();
  }
}

void loop() {
  // The ISR only counts presses; the actual work happens here, in normal
  // context, where neopixelWrite() and Serial are safe to call. Every press is
  // serviced, so holding two rapid presses gives two toggles.
  uint32_t presses = takePendingPresses();
  for (uint32_t i = 0; i < presses; i++) {
    toggleLed();
    Serial.printf("button: toggled, led now %s (total %lu)\n",
                  ledStatus ? "on" : "off", toggleCount);
  }

  if (wifiReady) {
    handleWifi();
  }
}