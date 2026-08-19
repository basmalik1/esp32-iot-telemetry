#include "status_json.h"

#include <stdio.h>

int statusJsonFormat(char *out, size_t size, const StatusFields &f) {
  return snprintf(out, size,
                  "{\"led\":%s,\"toggles\":%lu,\"rssi\":%d,\"uptime_ms\":%lu,"
                  "\"free_heap\":%lu,\"temp_c\":%.1f}\n",
                  f.ledOn ? "true" : "false", (unsigned long)f.toggles, f.rssi,
                  (unsigned long)f.uptimeMs, (unsigned long)f.freeHeap,
                  (double)f.tempC);
}
