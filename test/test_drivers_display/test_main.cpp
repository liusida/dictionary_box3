#include <Arduino.h>
#include <unity.h>
#include "display_manager.h"
#include "memory_test_helper.h"

using namespace dict;

// What are tested here:
// test_display.cpp
// Initialize: display manager initializes and reports ready.
void test_display_initialize_and_ready(void);
// Tick safety: tick() runs without crashing when ready.
void test_display_tick_when_ready_is_safe(void);
// Backlight: setBacklight toggles pin without error (smoke test).
void test_display_set_backlight_smoke(void);
// LVGL tick callback: returns a monotonic value (millis proxy).
void test_display_lvgl_tick_callback_monotonic(void);

// test_lvgl.cpp
// LVGL create/load screen: basic screen creation and load works.
void test_lvgl_create_and_load_screen(void);
// LVGL label create/set text: label is created and text is set.
void test_lvgl_create_label_and_set_text(void);
// LVGL display PNG image: PNG image is displayed correctly.
void test_lvgl_display_png_image(void);
// getDefaultGroup: creates and returns default LVGL group
void test_lvgl_helper_getDefaultGroup(void);
// loadScreen: loads screen and clears group
void test_lvgl_helper_loadScreen(void);
// addObjectToDefaultGroup: adds objects to default group
void test_lvgl_helper_addObjectToDefaultGroup(void);
// Integration test: tests all helper functions together
void test_lvgl_helper_integration(void);
// Multiple init/shutdown test: tests DisplayManager initialization and shutdown cycles
void test_lvgl_helper_multiple_init_shutdown(void);

#define TAG "DisplayTest"

// Start Test Suite
void setUp(void) {
    // Record memory state before each test
    setUpMemoryMonitoring();
}

void tearDown(void) {
    // Turn off display backlight after each test to save power
    DisplayManager disp;
    if (disp.isReady()) {
        disp.setBacklight(false);
    }
    
    // Check for memory leaks after each test
    tearDownMemoryMonitoring("test");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("Display", true);
    
    UNITY_BEGIN();
    
    // Display Manager Tests
    RUN_TEST_EX(TAG, test_display_initialize_and_ready);
    RUN_TEST_EX(TAG, test_display_tick_when_ready_is_safe);
    RUN_TEST_EX(TAG, test_display_set_backlight_smoke);
    RUN_TEST_EX(TAG, test_display_lvgl_tick_callback_monotonic);

    // LVGL Core Tests
    RUN_TEST_EX(TAG, test_lvgl_create_and_load_screen);
    RUN_TEST_EX(TAG, test_lvgl_create_label_and_set_text);
    RUN_TEST_EX(TAG, test_lvgl_display_png_image);

    // LVGL Helper Tests
    RUN_TEST_EX(TAG, test_lvgl_helper_getDefaultGroup);
    RUN_TEST_EX(TAG, test_lvgl_helper_loadScreen);
    RUN_TEST_EX(TAG, test_lvgl_helper_addObjectToDefaultGroup);
    RUN_TEST_EX(TAG, test_lvgl_helper_integration);
    RUN_TEST_EX(TAG, test_lvgl_helper_multiple_init_shutdown);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("Display", false);
}

void loop() {}