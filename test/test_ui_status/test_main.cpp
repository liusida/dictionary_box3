#include <Arduino.h>
#include <unity.h>
#include "ui_status.h"
#include "display_manager.h"
#include "memory_test_helper.h"

using namespace dict;

// What are tested here:
// Initialize: status overlay initializes and reports ready.
void test_status_overlay_initialize_and_ready(void);
// Attach/detach: attachToScreen and detachFromScreen work correctly.
void test_status_overlay_attach_detach_screen(void);
// Show/hide: show() and hide() toggle visibility correctly.
void test_status_overlay_show_hide(void);
// Status updates: updateWiFiStatus, updateBLEStatus, updateAudioStatus work.
void test_status_overlay_status_updates(void);
// Position: setPosition works correctly.
void test_status_overlay_set_position(void);
// Configuration: setIndicatorSize and setAnimationDuration work.
void test_status_overlay_configuration(void);
// Integration: test with real display manager.
void test_status_overlay_integration_with_display(void);

#define TAG "StatusOverlayTest"

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
    printTestSuiteMemorySummary("StatusOverlay", true);
    
    UNITY_BEGIN();
    
    // Status Overlay Tests
    RUN_TEST_EX(TAG, test_status_overlay_initialize_and_ready);
    RUN_TEST_EX(TAG, test_status_overlay_attach_detach_screen);
    RUN_TEST_EX(TAG, test_status_overlay_show_hide);
    RUN_TEST_EX(TAG, test_status_overlay_status_updates);
    RUN_TEST_EX(TAG, test_status_overlay_set_position);
    RUN_TEST_EX(TAG, test_status_overlay_configuration);
    RUN_TEST_EX(TAG, test_status_overlay_integration_with_display);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("StatusOverlay", false);
}

void loop() {}
