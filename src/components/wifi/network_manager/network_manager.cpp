#include "network_manager.h"

#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h" // WIFI_SSID, WIFI_PASSWORD - gitignored, see README

namespace {

constexpr uint32_t CONNECT_TIMEOUT_MS = 30000;
constexpr char HOSTNAME[] = "esp32-iot-telemetry";

bool connected = false;

// The ESP32-S3 radio is 2.4 GHz only, so a 5 GHz network is simply absent
// here. That absence is usually the answer.
void logVisibleNetworks() {
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
    Serial.printf("  %-32s ch%-3d %4d dBm\n", WiFi.SSID(i).c_str(),
                  WiFi.channel(i), WiFi.RSSI(i));
  }
  WiFi.scanDelete();
}

} // namespace

bool networkConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME); // must precede begin()
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.printf("wifi: connecting to %s", WIFI_SSID);

  // begin() returns immediately; the association completes in the background.
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("wifi: failed after %lu ms, status %d\n", millis() - start,
                  WiFi.status());
    logVisibleNetworks();
    connected = false;
    return false;
  }

  Serial.print("wifi: connected, IP ");
  Serial.println(WiFi.localIP());
  Serial.printf("wifi: rssi %d dBm\n", WiFi.RSSI());

  // Modem sleep parks the radio between beacons, which showed up as 80-300 ms
  // round trips. Disabling it costs 20-30 mA and makes HTTP feel immediate.
  WiFi.setSleep(false);

  connected = true;
  return true;
}

bool networkIsUp() { return connected; }

IPAddress networkIp() { return WiFi.localIP(); }

int networkRssi() { return WiFi.RSSI(); }
