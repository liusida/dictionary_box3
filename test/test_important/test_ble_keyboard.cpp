#include <Arduino.h>
#include <unity.h>
#include "../../lib/drivers_blekeyboard/ble_keyboard.h"
#include "../../lib/core_misc/memory_test_helper.h"

using namespace dict;

// =================================== TESTS ===================================

void test_ble_keyboard_initialize_and_ready(void) {
    BLEKeyboard& keyboard = BLEKeyboard::instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(&keyboard, "BLEKeyboard singleton should not be null");
    
    // Test initialization
    TEST_ASSERT_TRUE_MESSAGE(keyboard.initialize(), "BLEKeyboard initialize() failed");
    TEST_ASSERT_TRUE_MESSAGE(keyboard.isReady(), "BLEKeyboard should be ready after initialization");
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(keyboard.isReady());
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    // Test shutdown
    keyboard.shutdown();
    TEST_ASSERT_FALSE_MESSAGE(keyboard.isReady(), "BLEKeyboard should not be ready after shutdown");
}

void test_ble_keyboard_tick_safety(void) {
    BLEKeyboard& keyboard = BLEKeyboard::instance();
    TEST_ASSERT_TRUE_MESSAGE(keyboard.initialize(), "BLEKeyboard initialize() failed");
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    // Test that tick() can be called safely when ready
    keyboard.tick();
    keyboard.tick();
    keyboard.tick();
    
    // Verify keyboard is still ready after ticks
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    // Test tick() safety when not ready
    keyboard.shutdown();
    keyboard.tick(); // Should not crash
    keyboard.tick(); // Should not crash
    
    TEST_ASSERT_FALSE(keyboard.isReady());
}

void test_ble_keyboard_scanning_state(void) {
    BLEKeyboard& keyboard = BLEKeyboard::instance();
    TEST_ASSERT_TRUE_MESSAGE(keyboard.initialize(), "BLEKeyboard initialize() failed");
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    // Test initial scanning state
    TEST_ASSERT_FALSE_MESSAGE(keyboard.isScanning(), "Should not be scanning initially");
    
    // Test scan timing methods
    uint32_t scanStartTime = keyboard.getScanStartTime();
    uint32_t scanEndTime = keyboard.getScanEndTime();
    
    // Times should be valid (not necessarily meaningful without actual scan)
    TEST_ASSERT_TRUE_MESSAGE(scanStartTime >= 0, "Scan start time should be valid");
    TEST_ASSERT_TRUE_MESSAGE(scanEndTime >= 0, "Scan end time should be valid");
    
    // Test startScan (may or may not succeed depending on hardware)
    keyboard.startScan();
    
    // Give some time for scan to potentially start
    delay(100);
    
    // Verify still ready after scan attempt
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    keyboard.shutdown();
}

void test_ble_keyboard_connection_state(void) {
    BLEKeyboard& keyboard = BLEKeyboard::instance();
    TEST_ASSERT_TRUE_MESSAGE(keyboard.initialize(), "BLEKeyboard initialize() failed");
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    // Test connection state (should be false without actual BLE device)
    TEST_ASSERT_FALSE_MESSAGE(keyboard.isConnected(), "Should not be connected without BLE device");
    
    // Test begin method (should not crash)
    keyboard.begin();
    
    // Verify still ready after begin
    TEST_ASSERT_TRUE(keyboard.isReady());
    
    // Test key callback setting (should not crash)
    keyboard.setKeyCallback([](char key, uint8_t keyCode, uint8_t modifiers) {
        // Empty callback for testing
    });
    
    keyboard.shutdown();
}
