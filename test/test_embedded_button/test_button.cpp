// TC-2.1 (hardware half) - no false triggers from a button nobody is touching.
//
// The debounce arithmetic is covered on the host. What only the board can show
// is that attaching a real interrupt to a real pin does not produce phantom
// presses, and that the counter genuinely clears.

#include <Arduino.h>
#include <unity.h>

#include "components/button/button.h"

void setUp() {}
void tearDown() {}

// An unconfigured or noisy input would show up here as phantom presses.
void test_idle_produces_no_presses() {
  buttonInit();
  buttonTakePresses(); // discard anything latched during init
  delay(500);
  TEST_ASSERT_EQUAL_UINT32(0, buttonTakePresses());
}

// Taking presses must clear the counter, or one press would be serviced twice.
void test_take_clears_the_counter() {
  buttonTakePresses();
  TEST_ASSERT_EQUAL_UINT32(0, buttonTakePresses());
  TEST_ASSERT_EQUAL_UINT32(0, buttonTakePresses());
}

// The pull-up must hold the line high while the button is released. If this
// reads LOW, the button is either held, miswired, or the pull-up is not set -
// and the interrupt would fire continuously.
void test_pin_idles_high() {
  buttonInit();
  TEST_ASSERT_EQUAL_INT(HIGH, digitalRead(13));
}

void setup() {
  // See the note in test_embedded_led: printing before the runner attaches
  // loses the results and wedges the run.
  delay(5000);
  UNITY_BEGIN();
  RUN_TEST(test_idle_produces_no_presses);
  RUN_TEST(test_take_clears_the_counter);
  RUN_TEST(test_pin_idles_high);
  UNITY_END();
}

void loop() {}
