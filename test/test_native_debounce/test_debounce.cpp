// TC-2.1 - the debounce reports one clean press and no false triggers.
//
// Runs on the host: the decision is a pure function, so no board is involved.
// That is what makes the millis() rollover case reachable - hitting it on real
// hardware would take 49.7 days of uptime.

#include <unity.h>

#include "components/button/debounce.h"

constexpr uint32_t DEBOUNCE = 50;

void setUp() {}
void tearDown() {}

void test_first_edge_is_accepted() {
  // lastAccepted = 0 is the state at boot, before any press.
  TEST_ASSERT_TRUE(buttonDebounceAccepts(1000, 0, DEBOUNCE));
}

void test_bounce_within_window_is_rejected() {
  TEST_ASSERT_FALSE(buttonDebounceAccepts(1002, 1000, DEBOUNCE));
  TEST_ASSERT_FALSE(buttonDebounceAccepts(1049, 1000, DEBOUNCE));
}

void test_edge_exactly_at_window_is_accepted() {
  // The boundary is inclusive: >= debounceMs, not >.
  TEST_ASSERT_TRUE(buttonDebounceAccepts(1050, 1000, DEBOUNCE));
}

void test_edge_after_window_is_accepted() {
  TEST_ASSERT_TRUE(buttonDebounceAccepts(2000, 1000, DEBOUNCE));
}

// A real press bounces several times over a few milliseconds. Only the first
// edge may be accepted; every bounce after it must be rejected.
void test_realistic_bounce_burst_yields_one_press() {
  const uint32_t edges[] = {1000, 1001, 1003, 1004, 1007, 1012};
  uint32_t lastAccepted = 0;
  int accepted = 0;

  for (uint32_t edge : edges) {
    if (buttonDebounceAccepts(edge, lastAccepted, DEBOUNCE)) {
      accepted++;
      lastAccepted = edge;
    }
  }
  TEST_ASSERT_EQUAL_INT(1, accepted);
}

// Two deliberate presses far enough apart must both register - the filter must
// not be so aggressive that it swallows real input.
void test_two_deliberate_presses_both_accepted() {
  const uint32_t edges[] = {1000, 1001, 1200, 1201}; // press, bounce, press, bounce
  uint32_t lastAccepted = 0;
  int accepted = 0;

  for (uint32_t edge : edges) {
    if (buttonDebounceAccepts(edge, lastAccepted, DEBOUNCE)) {
      accepted++;
      lastAccepted = edge;
    }
  }
  TEST_ASSERT_EQUAL_INT(2, accepted);
}

// millis() wraps to zero after ~49.7 days. Unsigned subtraction keeps the
// elapsed time correct across that boundary.
void test_survives_millis_rollover() {
  const uint32_t justBeforeWrap = 0xFFFFFFF0; // 16 ms short of wrapping

  // 40 ms after the wrap is 56 ms of real elapsed time: a genuine press.
  TEST_ASSERT_TRUE(buttonDebounceAccepts(40, justBeforeWrap, DEBOUNCE));

  // 4 ms after the wrap is only 20 ms elapsed: still bounce.
  TEST_ASSERT_FALSE(buttonDebounceAccepts(4, justBeforeWrap, DEBOUNCE));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_first_edge_is_accepted);
  RUN_TEST(test_bounce_within_window_is_rejected);
  RUN_TEST(test_edge_exactly_at_window_is_accepted);
  RUN_TEST(test_edge_after_window_is_accepted);
  RUN_TEST(test_realistic_bounce_burst_yields_one_press);
  RUN_TEST(test_two_deliberate_presses_both_accepted);
  RUN_TEST(test_survives_millis_rollover);
  return UNITY_END();
}
