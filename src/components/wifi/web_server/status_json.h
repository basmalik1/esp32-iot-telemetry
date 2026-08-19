#pragma once

#include <stddef.h>
#include <stdint.h>

// Everything /status reports. Grouped into a struct rather than a long
// parameter list so adding a field does not silently reorder the call sites.
struct StatusFields {
  bool ledOn;
  uint32_t toggles;
  int rssi;
  uint32_t uptimeMs;
  uint32_t freeHeap;
  float tempC;
};

// Renders the JSON body every endpoint replies with. Kept free of Arduino and
// WebServer dependencies so the payload format can be unit-tested on the host,
// with no board and no network.
//
// Writes into a caller-owned buffer rather than building an Arduino String: no
// heap allocation on a path that runs thousands of times, and the size
// argument makes overflow impossible.
//
// Returns what snprintf returns - the length the output would have had, which
// exceeds `size` if the buffer was too small.
int statusJsonFormat(char *out, size_t size, const StatusFields &f);
