#include <Arduino.h>
#include <unity.h>
#include "../../lib/drivers_audio/audio_manager.h"
#include "../../lib/core_misc/memory_test_helper.h"
#include "LittleFS.h"

using namespace dict;

// =================================== TESTS ===================================

void test_audio_manager_initialize_and_ready(void) {
    AudioManager& manager = AudioManager::instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(&manager, "AudioManager singleton should not be null");
    
    // Test initialization
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "AudioManager initialize() failed");
    TEST_ASSERT_TRUE_MESSAGE(manager.isReady(), "AudioManager should be ready after initialization");
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(manager.isReady());
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test shutdown
    manager.shutdown();
    TEST_ASSERT_FALSE_MESSAGE(manager.isReady(), "AudioManager should not be ready after shutdown");
}

void test_audio_manager_tick_safety(void) {
    AudioManager& manager = AudioManager::instance();
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "AudioManager initialize() failed");
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test that tick() can be called safely when ready
    manager.tick();
    manager.tick();
    manager.tick();
    
    // Verify audio is still ready after ticks
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test tick() safety when not ready
    manager.shutdown();
    manager.tick(); // Should not crash
    manager.tick(); // Should not crash
    
    TEST_ASSERT_FALSE(manager.isReady());
}

void test_audio_manager_play_stop(void) {
    AudioManager& manager = AudioManager::instance();
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "AudioManager initialize() failed");
    TEST_ASSERT_TRUE(manager.isReady());
       
    // Test stop functionality (should not crash even when not playing)
    TEST_ASSERT_TRUE_MESSAGE(manager.stop(), "Stop should succeed even when not playing");
    
    // Test play with invalid URL (should handle gracefully)
    TEST_ASSERT_FALSE_MESSAGE(manager.play(nullptr), "Play with null URL should fail");
    TEST_ASSERT_FALSE_MESSAGE(manager.play(""), "Play with empty URL should fail");
    
    // Verify still ready after operations
    TEST_ASSERT_TRUE(manager.isReady());
    
    manager.shutdown();
}

void test_audio_manager_volume_control(void) {
    AudioManager& manager = AudioManager::instance();
    TEST_ASSERT_TRUE_MESSAGE(manager.initialize(), "AudioManager initialize() failed");
    TEST_ASSERT_TRUE(manager.isReady());
    
    // Test volume setting
    manager.setVolume(0.0f);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.0f, manager.getVolume(), "Volume should be 0.0");
    
    manager.setVolume(0.5f);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.5f, manager.getVolume(), "Volume should be 0.5");
    
    manager.setVolume(1.0f);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(1.0f, manager.getVolume(), "Volume should be 1.0");
    
    // Test edge cases
    manager.setVolume(-0.1f); // Should clamp to 0.0
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.0f, manager.getVolume(), "Negative volume should clamp to 0.0");
    
    manager.setVolume(1.1f); // Should clamp to 1.0
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(1.0f, manager.getVolume(), "Volume > 1.0 should clamp to 1.0");
    
    // Verify still ready after volume changes
    TEST_ASSERT_TRUE(manager.isReady());
    
    manager.shutdown();
}
