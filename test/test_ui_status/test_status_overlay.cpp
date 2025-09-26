#include <Arduino.h>
#include <unity.h>
#include "ui_status.h"
#include "display_manager.h"

using namespace dict;

// =================================== TESTS ===================================

void test_status_overlay_initialize_and_ready(void) {
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(status.isReady());
    TEST_ASSERT_TRUE(status.isReady());
    
    // Explicitly shutdown StatusOverlay
    status.shutdown();
    TEST_ASSERT_FALSE(status.isReady());
}

void test_status_overlay_attach_detach_screen(void) {
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Create a test screen
    lv_obj_t* testScreen = lv_obj_create(NULL);
    TEST_ASSERT_NOT_NULL(testScreen);
    
    // Test initial state
    TEST_ASSERT_FALSE(status.isAttached());
    
    // Test attach
    status.attachToScreen(testScreen);
    TEST_ASSERT_TRUE(status.isAttached());
    
    // Test detach
    status.detachFromScreen();
    TEST_ASSERT_FALSE(status.isAttached());
    
    // Clean up
    lv_obj_del(testScreen);
    status.shutdown();
}

void test_status_overlay_show_hide(void) {
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Create a test screen and attach
    lv_obj_t* testScreen = lv_obj_create(NULL);
    status.attachToScreen(testScreen);
    
    // Test initial visibility (should be hidden by default)
    TEST_ASSERT_FALSE(status.isVisible());
    
    // Test show
    status.show();
    TEST_ASSERT_TRUE(status.isVisible());
    
    // Test hide
    status.hide();
    TEST_ASSERT_FALSE(status.isVisible());
    
    // Test show again
    status.show();
    TEST_ASSERT_TRUE(status.isVisible());
    
    // Clean up
    lv_obj_del(testScreen);
    status.shutdown();
}

void test_status_overlay_status_updates(void) {
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Test WiFi status updates
    status.updateWiFiStatus(false);
    status.updateWiFiStatus(true, "TestWiFi");
    
    // Test BLE status updates
    status.updateBLEStatus(false);
    status.updateBLEStatus(true, "TestKeyboard");
    
    // Test Audio status updates
    status.updateAudioStatus(false);
    status.updateAudioStatus(true, "test.mp3");
    
    // Test tick() doesn't crash
    status.tick();
    status.tick();
    status.tick();
    
    // Verify status overlay is still ready
    TEST_ASSERT_TRUE(status.isReady());
    
    status.shutdown();
}

void test_status_overlay_set_position(void) {
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Create a test screen and attach
    lv_obj_t* testScreen = lv_obj_create(NULL);
    status.attachToScreen(testScreen);
    
    // Test different positions
    status.setPosition(LV_ALIGN_TOP_LEFT, 10, 10);
    status.setPosition(LV_ALIGN_TOP_RIGHT, -10, 10);
    status.setPosition(LV_ALIGN_BOTTOM_LEFT, 10, -10);
    status.setPosition(LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    status.setPosition(LV_ALIGN_CENTER, 0, 0);
    
    // Verify status overlay is still ready
    TEST_ASSERT_TRUE(status.isReady());
    
    // Clean up
    lv_obj_del(testScreen);
    status.shutdown();
}

void test_status_overlay_configuration(void) {
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Test indicator size changes
    status.setIndicatorSize(15);
    status.setIndicatorSize(25);
    status.setIndicatorSize(30);
    
    // Test animation duration changes
    status.setAnimationDuration(100);
    status.setAnimationDuration(500);
    status.setAnimationDuration(1000);
    
    // Test tick() doesn't crash with new settings
    status.tick();
    
    // Verify status overlay is still ready
    TEST_ASSERT_TRUE(status.isReady());
    
    status.shutdown();
}

void test_status_overlay_integration_with_display(void) {
    // Initialize display manager
    DisplayManager display;
    TEST_ASSERT_TRUE_MESSAGE(display.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(display.isReady());
    
    // Initialize status overlay
    StatusOverlay status;
    TEST_ASSERT_TRUE_MESSAGE(status.initialize(), "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());
    
    // Create a test screen
    lv_obj_t* testScreen = lv_obj_create(NULL);
    TEST_ASSERT_NOT_NULL(testScreen);
    lv_disp_load_scr(testScreen);
    
    // Attach status overlay to the active screen
    status.attachToScreen(testScreen);
    status.show();
    
    // Update status indicators
    status.updateWiFiStatus(true, "TestWiFi");
    status.updateBLEStatus(true, "TestKeyboard");
    status.updateAudioStatus(false);
    
    // Set position
    status.setPosition(LV_ALIGN_TOP_RIGHT, -10, 10);
    
    // Run display tick to process LVGL
    display.tick();
    
    // Verify everything is still working
    TEST_ASSERT_TRUE(display.isReady());
    TEST_ASSERT_TRUE(status.isReady());
    TEST_ASSERT_TRUE(status.isAttached());
    TEST_ASSERT_TRUE(status.isVisible());
    
    // Clean up
    lv_obj_del(testScreen);
    status.shutdown();
    display.shutdown();
}
