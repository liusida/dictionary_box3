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
// Manual visual check: wait for manual visual check.
void test_status_overlay_manual_visual_check(void);
// Simple LVGL drawing test
void test_draw_without_status_overlay(void);

void test_minimal_direct_create(void);
void test_minimal_create_attach(void);

#define TAG "StatusOverlayTest"

// Global display manager for all tests
DisplayManager* g_display = nullptr;

// Start Test Suite
void setUp(void) {
    // Record memory state before each test
    setUpMemoryMonitoring();
    
    // Initialize display manager if not already done
    if (!g_display) {
        g_display = new DisplayManager();
        TEST_ASSERT_TRUE_MESSAGE(g_display->initialize(), "Display initialize() failed in setUp");
        TEST_ASSERT_TRUE(g_display->isReady());
    } else {
        // Turn on display for testing
        g_display->setBacklight(true);
    }
}

void tearDown(void) {
    // Turn off display backlight after each test to save power
    if (g_display && g_display->isReady()) {
        g_display->setBacklight(false);
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
    // RUN_TEST_EX(TAG, test_status_overlay_initialize_and_ready);
    // RUN_TEST_EX(TAG, test_status_overlay_attach_detach_screen);
    // RUN_TEST_EX(TAG, test_status_overlay_show_hide);
    // RUN_TEST_EX(TAG, test_status_overlay_status_updates);
    // RUN_TEST_EX(TAG, test_status_overlay_set_position);
    // RUN_TEST_EX(TAG, test_status_overlay_configuration);
    // RUN_TEST_EX(TAG, test_status_overlay_integration_with_display);
    // RUN_TEST_EX(TAG, test_draw_without_status_overlay);
    RUN_TEST_EX(TAG, test_status_overlay_manual_visual_check);
    // RUN_TEST_EX(TAG, test_minimal_create_attach);
    // RUN_TEST_EX(TAG, test_minimal_direct_create);

    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("StatusOverlay", false);
    
    // Clean up global display
    if (g_display) {
        g_display->shutdown();
        delete g_display;
        g_display = nullptr;
    }
}

void loop() {}
