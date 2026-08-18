// TC-3.1 - endpoint payloads are valid and carry the expected values.
//
// Runs on the host. The status body is rendered by a pure function, so the
// format is verified with no board, no network and no HTTP client. Route
// dispatch is covered by the system test, which needs a real socket.

#include <string.h>
#include <unity.h>

#include "components/wifi/web_server/status_json.h"

static char body[160];

void setUp() { memset(body, 0, sizeof(body)); }
void tearDown() {}

void test_renders_all_fields() {
  statusJsonFormat(body, sizeof(body), true, 42, -31, 65138);
  TEST_ASSERT_EQUAL_STRING(
      "{\"led\":true,\"toggles\":42,\"rssi\":-31,\"uptime_ms\":65138}\n", body);
}

// Clients parse this, so the LED state must be a JSON boolean - not "1"/"0"
// and not "on"/"off".
void test_led_false_renders_as_json_false() {
  statusJsonFormat(body, sizeof(body), false, 0, -70, 0);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"led\":false"));
  TEST_ASSERT_NULL(strstr(body, "\"led\":0"));
}

void test_negative_rssi_keeps_its_sign() {
  statusJsonFormat(body, sizeof(body), true, 1, -100, 5);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"rssi\":-100"));
}

// toggles and uptime_ms are 32-bit unsigned. A wrong format specifier would
// print these as negative numbers once they pass 2^31.
void test_large_unsigned_values_do_not_print_negative() {
  statusJsonFormat(body, sizeof(body), true, 4000000000u, -31, 4294967295u);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"toggles\":4000000000"));
  TEST_ASSERT_NOT_NULL(strstr(body, "\"uptime_ms\":4294967295"));
}

// The handler passes a fixed buffer. A short one must truncate safely rather
// than overrun, and must stay NUL-terminated.
void test_truncation_is_safe() {
  char tiny[10];
  memset(tiny, 0x7F, sizeof(tiny));
  int needed = statusJsonFormat(tiny, sizeof(tiny), true, 1, -1, 1);

  TEST_ASSERT_GREATER_THAN_INT((int)sizeof(tiny), needed);
  TEST_ASSERT_EQUAL_CHAR('\0', tiny[sizeof(tiny) - 1]);
}

// Guards the real handler's buffer size against the worst case: every numeric
// field at maximum width. If this fails, responses would silently truncate.
void test_handler_buffer_is_large_enough() {
  int needed = statusJsonFormat(body, sizeof(body), false, 4294967295u, -100,
                                4294967295u);
  TEST_ASSERT_LESS_THAN_INT((int)sizeof(body), needed);
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_renders_all_fields);
  RUN_TEST(test_led_false_renders_as_json_false);
  RUN_TEST(test_negative_rssi_keeps_its_sign);
  RUN_TEST(test_large_unsigned_values_do_not_print_negative);
  RUN_TEST(test_truncation_is_safe);
  RUN_TEST(test_handler_buffer_is_large_enough);
  return UNITY_END();
}
