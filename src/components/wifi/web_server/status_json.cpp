#include "status_json.h"

#include <stdio.h>

int statusJsonFormat(char *out, size_t size, bool ledOn, uint32_t toggles,
                     int rssi, uint32_t uptimeMs) {
  return snprintf(out, size,
                  "{\"led\":%s,\"toggles\":%lu,\"rssi\":%d,\"uptime_ms\":%lu}\n",
                  ledOn ? "true" : "false", (unsigned long)toggles, rssi,
                  (unsigned long)uptimeMs);
}
