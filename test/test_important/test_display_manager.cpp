#include <Arduino.h>
#include <unity.h>
#include "../../lib/drivers_display/display_manager.h"
#include "../../lib/drivers_i2c/i2c_manager.h"
#include "../../lib/core_misc/memory_test_helper.h"

using namespace dict;

// =================================== TESTS ===================================

void test_display_manager_initialize_and_ready(void) {
    // Test singleton access
    DisplayManager& manager = DisplayManager::instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(&manager, "DisplayManager singleton should not be null");
    
    // Test initialization
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "DisplayManager initialize() failed");
    TEST_ASSERT_TRUE_MESSAGE(manager.isReady(), "DisplayManager should be ready after initialization");
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(manager.isReady());
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test shutdown
    manager.shutdown();
    TEST_ASSERT_FALSE_MESSAGE(manager.isReady(), "DisplayManager should not be ready after shutdown");
}

void test_display_manager_tick_safety(void) {
    DisplayManager& manager = DisplayManager::instance();
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "DisplayManager initialize() failed");
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test that tick() can be called safely when ready
    manager.tick();
    manager.tick();
    manager.tick();
    
    // Verify display is still ready after ticks
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test tick() safety when not ready
    manager.shutdown();
    manager.tick(); // Should not crash
    manager.tick(); // Should not crash
    
    TEST_ASSERT_FALSE(manager.isReady());
}

void test_display_manager_reset_display(void) {
    DisplayManager& manager = DisplayManager::instance();
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "DisplayManager initialize() failed");
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test reset display functionality
    manager.resetDisplay();
    
    // Verify display is still ready after reset
    TEST_ASSERT_TRUE(manager.isReady());
    
    manager.shutdown();
}

void test_display_manager_backlight_control(void) {
    DisplayManager& manager = DisplayManager::instance();
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "DisplayManager initialize() failed");
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test backlight control
    manager.setBacklight(true);
    manager.setBacklight(false);
    manager.setBacklight(true);
    
    // Verify display is still ready after backlight changes
    TEST_ASSERT_TRUE(manager.isReady());
    
    manager.shutdown();
}
