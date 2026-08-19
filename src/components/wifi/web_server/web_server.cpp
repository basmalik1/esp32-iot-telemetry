#include "web_server.h"

#include <Arduino.h>
#include <WebServer.h>

#include "components/wifi/network_manager/network_manager.h"
#include "components/wifi/web_server/status_json.h"
#include "components/wifi/web_server/web_ui.h"

namespace {

WebServer server(80); // port 80 so URLs need no :port suffix
LedControl led{};

// Every endpoint answers with the resulting state, so a caller never needs a
// second request to find out what happened.
void sendStatus() {
  char body[192];
  StatusFields f{led.isOn(),        led.toggles(),      networkRssi(),
                 millis(),          ESP.getFreeHeap(),  temperatureRead()};
  statusJsonFormat(body, sizeof(body), f);
  server.send(200, "application/json", body);
}

// The dashboard, compiled into the binary and already gzipped. send_P streams
// it straight out of flash rather than copying 80 kB into RAM first.
void handleRoot() {
  server.sendHeader("Content-Encoding", "gzip");
  server.send_P(200, "text/html", reinterpret_cast<PGM_P>(WEB_UI_GZ),
                WEB_UI_GZ_LEN);
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
