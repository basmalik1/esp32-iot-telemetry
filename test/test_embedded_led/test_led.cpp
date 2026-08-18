// TC-1.1 - the LED can be turned on, turned off, and toggled.
//
// Runs on the board. These calls go through the real digitalWrite() and
// neopixelWrite(), so a pass means the driver path works rather than just the
// bookkeeping - which is the part a host test could never tell you.

#include <Arduino.h>
#include <unity.h>

#include "components/led/led.h"

void setUp() { ledOff(); }
void tearDown() { ledOff(); }

void test_init_leaves_led_off() {
  ledInit();
  TEST_ASSERT_FALSE(ledIsOn());
}

void test_on_then_off() {
  ledOn();
  TEST_ASSERT_TRUE(ledIsOn());
  ledOff();
  TEST_ASSERT_FALSE(ledIsOn());
}

void test_on_is_idempotent() {
  ledOn();
  ledOn();
  TEST_ASSERT_TRUE(ledIsOn());
}

void test_toggle_alternates() {
  TEST_ASSERT_FALSE(ledIsOn());
  ledToggle();
  TEST_ASSERT_TRUE(ledIsOn());
  ledToggle();
  TEST_ASSERT_FALSE(ledIsOn());
}

// TC-3.3 relies on this counter to prove nothing was dropped, so it must count
// toggles specifically - not every state change.
void test_toggle_count_ignores_on_and_off() {
  uint32_t before = ledToggleCount();

  ledOn();
  ledOff();
  TEST_ASSERT_EQUAL_UINT32(before, ledToggleCount());

  ledToggle();
  TEST_ASSERT_EQUAL_UINT32(before + 1, ledToggleCount());
}

void test_toggle_count_tracks_many_toggles() {
  uint32_t before = ledToggleCount();
  for (int i = 0; i < 50; i++) {
    ledToggle();
  }
  TEST_ASSERT_EQUAL_UINT32(before + 50, ledToggleCount());
  TEST_ASSERT_FALSE(ledIsOn()); // 50 is even, so it ends where it started
}

void setup() {
  // Wait for the test runner to attach before printing anything. After
  // flashing, esptool resets the board and the firmware starts immediately,
  // while PlatformIO still has to close esptool's port and open its own. Print
  // into that gap and the results are lost - the runner then waits forever for
  // output that already happened. 2 s was not enough; 5 s is.
  delay(5000);
  UNITY_BEGIN();
  RUN_TEST(test_init_leaves_led_off);
  RUN_TEST(test_on_then_off);
  RUN_TEST(test_on_is_idempotent);
  RUN_TEST(test_toggle_alternates);
  RUN_TEST(test_toggle_count_ignores_on_and_off);
  RUN_TEST(test_toggle_count_tracks_many_toggles);
  UNITY_END();
}

void loop() {}
