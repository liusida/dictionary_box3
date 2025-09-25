#include <Arduino.h>
#include <unity.h>
#include "display_manager.h"

using namespace dict;

// =================================== TESTS ===================================
void test_display_initialize_and_ready(void) {
  DisplayManager disp;
  TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
  TEST_ASSERT_TRUE(disp.isReady());
}

void test_display_tick_when_ready_is_safe(void) {
  DisplayManager disp;
  if (!disp.isReady()) { (void)disp.initialize(); }
  disp.tick();
  TEST_ASSERT_TRUE(disp.isReady());
}

void test_display_set_backlight_smoke(void) {
  DisplayManager disp;
  if (!disp.isReady()) { (void)disp.initialize(); }
  disp.setBacklight(true);
  disp.setBacklight(false);
  TEST_ASSERT_TRUE(disp.isReady());
}

void test_display_lvgl_tick_callback_monotonic(void) {
  uint32_t a = DisplayManager::tickCallback();
  delay(5);
  uint32_t b = DisplayManager::tickCallback();
  TEST_ASSERT_TRUE(b >= a);
}