// TC-3.1 - endpoint payloads are valid and carry the expected values.
//
// Runs on the host. The status body is rendered by a pure function, so the
// format is verified with no board, no network and no HTTP client. Route
// dispatch is covered by the system test, which needs a real socket.

#include <string.h>
#include <unity.h>

#include "components/wifi/web_server/status_json.h"

static char body[192];

// A plausible reading, so each test only varies what it is actually about.
static StatusFields sample() {
  return StatusFields{true, 42, -31, 65138, 210000, 41.5f};
}

void setUp() { memset(body, 0, sizeof(body)); }
void tearDown() {}

void test_renders_all_fields() {
  statusJsonFormat(body, sizeof(body), sample());
  TEST_ASSERT_EQUAL_STRING(
      "{\"led\":true,\"toggles\":42,\"rssi\":-31,\"uptime_ms\":65138,"
      "\"free_heap\":210000,\"temp_c\":41.5}\n",
      body);
}

// Clients parse this, so the LED state must be a JSON boolean - not "1"/"0"
// and not "on"/"off".
void test_led_false_renders_as_json_false() {
  StatusFields f = sample();
  f.ledOn = false;
  statusJsonFormat(body, sizeof(body), f);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"led\":false"));
  TEST_ASSERT_NULL(strstr(body, "\"led\":0"));
}

void test_negative_rssi_keeps_its_sign() {
  StatusFields f = sample();
  f.rssi = -100;
  statusJsonFormat(body, sizeof(body), f);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"rssi\":-100"));
}

// Temperature is the one float in the payload. One decimal place, and a
// negative value must keep its sign rather than being formatted as unsigned.
void test_temperature_formats_to_one_decimal() {
  StatusFields f = sample();
  f.tempC = 23.456f;
  statusJsonFormat(body, sizeof(body), f);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"temp_c\":23.5"));

  f.tempC = -5.25f;
  statusJsonFormat(body, sizeof(body), f);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"temp_c\":-5.2"));
}

// toggles, uptime_ms and free_heap are 32-bit unsigned. A wrong format
// specifier would print these as negative numbers once they pass 2^31.
void test_large_unsigned_values_do_not_print_negative() {
  StatusFields f = sample();
  f.toggles = 4000000000u;
  f.uptimeMs = 4294967295u;
  f.freeHeap = 3000000000u;
  statusJsonFormat(body, sizeof(body), f);
  TEST_ASSERT_NOT_NULL(strstr(body, "\"toggles\":4000000000"));
  TEST_ASSERT_NOT_NULL(strstr(body, "\"uptime_ms\":4294967295"));
  TEST_ASSERT_NOT_NULL(strstr(body, "\"free_heap\":3000000000"));
}

// The handler passes a fixed buffer. A short one must truncate safely rather
// than overrun, and must stay NUL-terminated.
void test_truncation_is_safe() {
  char tiny[10];
  memset(tiny, 0x7F, sizeof(tiny));
  int needed = statusJsonFormat(tiny, sizeof(tiny), sample());

  TEST_ASSERT_GREATER_THAN_INT((int)sizeof(tiny), needed);
  TEST_ASSERT_EQUAL_CHAR('\0', tiny[sizeof(tiny) - 1]);
}

// Guards the real handler's buffer against the worst case: every numeric field
// at maximum width. If this fails, responses would silently truncate.
void test_handler_buffer_is_large_enough() {
  StatusFields f{false, 4294967295u, -100, 4294967295u, 4294967295u, -100.5f};
  int needed = statusJsonFormat(body, sizeof(body), f);
  TEST_ASSERT_LESS_THAN_INT((int)sizeof(body), needed);
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_renders_all_fields);
  RUN_TEST(test_led_false_renders_as_json_false);
  RUN_TEST(test_negative_rssi_keeps_its_sign);
  RUN_TEST(test_temperature_formats_to_one_decimal);
  RUN_TEST(test_large_unsigned_values_do_not_print_negative);
  RUN_TEST(test_truncation_is_safe);
  RUN_TEST(test_handler_buffer_is_large_enough);
  return UNITY_END();
}
