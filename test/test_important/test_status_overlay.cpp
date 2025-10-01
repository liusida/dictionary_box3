#include <Arduino.h>
#include <unity.h>
#include "../../lib/ui_status/ui_status.h"
#include "../../lib/drivers_display/display_manager.h"
#include "../../lib/core_misc/memory_test_helper.h"
#include "lvgl.h"

using namespace dict;

// =================================== TESTS ===================================

void test_status_overlay_initialize_and_ready(void) {
    StatusOverlay& overlay = StatusOverlay::instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(&overlay, "StatusOverlay singleton should not be null");
    
    // Test initialization
    TEST_ASSERT_TRUE_MESSAGE(overlay.initialize(), "StatusOverlay initialize() failed");
    TEST_ASSERT_TRUE_MESSAGE(overlay.isReady(), "StatusOverlay should be ready after initialization");
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(overlay.isReady());
    TEST_ASSERT_TRUE(overlay.isReady());
    
    // Test shutdown
    overlay.shutdown();
    TEST_ASSERT_FALSE_MESSAGE(overlay.isReady(), "StatusOverlay should not be ready after shutdown");
}

void test_status_overlay_attach_detach(void) {
    StatusOverlay& overlay = StatusOverlay::instance();
    TEST_ASSERT_TRUE_MESSAGE(overlay.initialize(), "StatusOverlay initialize() failed");
    TEST_ASSERT_TRUE(overlay.isReady());
    
    // Test initial state
    TEST_ASSERT_FALSE_MESSAGE(overlay.isAttached(), "Should not be attached initially");
    TEST_ASSERT_FALSE_MESSAGE(overlay.isVisible(), "Should not be visible initially");
    
    // Test attach to null screen (should handle gracefully)
    overlay.attachToScreen(nullptr);
    TEST_ASSERT_FALSE_MESSAGE(overlay.isAttached(), "Should not be attached to null screen");
    
    // Test detach when not attached (should not crash)
    overlay.detachFromScreen();
    TEST_ASSERT_FALSE_MESSAGE(overlay.isAttached(), "Should not be attached after detach");
    
    // Test show/hide when not attached
    overlay.show();
    overlay.hide();
    
    // Verify still ready after attach/detach operations
    TEST_ASSERT_TRUE(overlay.isReady());
    
    overlay.shutdown();
}

void test_status_overlay_status_updates(void) {
    StatusOverlay& overlay = StatusOverlay::instance();
    TEST_ASSERT_TRUE_MESSAGE(overlay.initialize(), "StatusOverlay initialize() failed");
    TEST_ASSERT_TRUE(overlay.isReady());
    
    // Test WiFi status updates
    overlay.updateWiFiStatus(WiFiState::None);
    overlay.updateWiFiStatus(WiFiState::Ready, "test_ssid");
    overlay.updateWiFiStatus(WiFiState::Working);
    
    // Test BLE status updates
    overlay.updateBLEStatus(false);
    overlay.updateBLEStatus(true, "test_device");
    
    // Test audio status updates
    overlay.updateAudioStatus(AudioState::None);
    overlay.updateAudioStatus(AudioState::Ready);
    overlay.updateAudioStatus(AudioState::Working, "test_track");
    
    // Verify still ready after status updates
    TEST_ASSERT_TRUE(overlay.isReady());
    
    overlay.shutdown();
}

void test_status_overlay_position_control(void) {
    StatusOverlay& overlay = StatusOverlay::instance();
    TEST_ASSERT_TRUE_MESSAGE(overlay.initialize(), "StatusOverlay initialize() failed");
    TEST_ASSERT_TRUE(overlay.isReady());
    
    // Test position setting (should not crash even without attached screen)
    overlay.setPosition(LV_ALIGN_TOP_LEFT, 10, 10);
    overlay.setPosition(LV_ALIGN_TOP_RIGHT, -10, 10);
    overlay.setPosition(LV_ALIGN_BOTTOM_LEFT, 10, -10);
    overlay.setPosition(LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    overlay.setPosition(LV_ALIGN_CENTER, 0, 0);
    
    // Test style setting (should not crash)
    overlay.setStyle(nullptr); // Null style should be handled gracefully
    
    // Test indicator size setting
    overlay.setIndicatorSize(16);
    overlay.setIndicatorSize(32);
    overlay.setIndicatorSize(8);
    
    // Test animation duration setting
    overlay.setAnimationDuration(100);
    overlay.setAnimationDuration(500);
    overlay.setAnimationDuration(1000);
    
    // Test container access
    lv_obj_t* container = overlay.getContainer();
    TEST_ASSERT_NOT_NULL_MESSAGE(container, "Container should not be null");
    
    // Verify still ready after position/style operations
    TEST_ASSERT_TRUE(overlay.isReady());
    
    overlay.shutdown();
}
