#pragma once

#include <stddef.h>
#include <stdint.h>

// What the server is allowed to do to the output it controls. Passing this in
// rather than including led.h keeps this module free of any concrete actuator:
// it serves whatever it is handed, so it can be reused unchanged in another
// project, and unit-tested against a fake with no hardware present.
struct LedControl {
  void (*on)();
  void (*off)();
  void (*toggle)();
  bool (*isOn)();
  uint32_t (*toggles)();
};

// Registers routes and opens port 80. Call only after the network is up.
void webServerBegin(const LedControl &led);

// Must run every loop() iteration - the server is synchronous, so a request
// sits unanswered until this is called. This is why loop() must never block.
void webServerPoll();
