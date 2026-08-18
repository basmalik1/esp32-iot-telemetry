#include "web_server.h"

#include <Arduino.h>
#include <WebServer.h>

#include "components/wifi/network_manager/network_manager.h"
#include "components/wifi/web_server/status_json.h"

namespace {

WebServer server(80); // port 80 so URLs need no :port suffix
LedControl led{};

// Every endpoint answers with the resulting state, so a caller never needs a
// second request to find out what happened.
void sendStatus() {
  char body[160];
  statusJsonFormat(body, sizeof(body), led.isOn(), led.toggles(), networkRssi(),
                   millis());
  server.send(200, "application/json", body);
}

// Plain links, so the board can be driven from a phone browser with no tooling.
void handleRoot() {
  static const char page[] =
      "<!doctype html><meta name=viewport content=\"width=device-width\">"
      "<h2>esp32-iot-telemetry</h2><p>"
      "<a href=\"/on\">on</a> | <a href=\"/off\">off</a> | "
      "<a href=\"/toggle\">toggle</a> | <a href=\"/status\">status</a></p>";
  server.send(200, "text/html", page);
}

void handleOn() {
  led.on();
  sendStatus();
}

void handleOff() {
  led.off();
  sendStatus();
}

void handleToggle() {
  led.toggle();
  sendStatus();
}

void handleNotFound() { server.send(404, "text/plain", "no such endpoint\n"); }

} // namespace

void webServerBegin(const LedControl &control) {
  led = control;

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/toggle", handleToggle);
  server.on("/status", sendStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.print("http: serving at http://");
  Serial.println(networkIp());
}

void webServerPoll() { server.handleClient(); }
