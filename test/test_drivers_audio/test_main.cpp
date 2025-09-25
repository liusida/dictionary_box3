#include <Arduino.h>
#include <unity.h>
#include "audio_manager.h"
#include "memory_test_helper.h"
#include "LittleFS.h"
#include "i2c_manager.h"

using namespace dict;

// What are tested here:
// test_audio.cpp
// Initialize: audio manager initializes and reports ready.
void test_audio_initialize_and_ready(void);
// Tick safety: tick() runs without crashing when ready.
void test_audio_tick_when_ready_is_safe(void);
// Volume: setVolume works without error (smoke test).
void test_audio_set_volume_smoke(void);
// Playback state: isCurrentlyPlaying and isCurrentlyPaused work correctly.
void test_audio_playback_state(void);
// Stop functionality: stop() works without crashing.
void test_audio_stop_functionality(void);
// MP3 playback: test actual MP3 file playback (requires WiFi).
void test_audio_mp3_playback(void);
// WiFi MP3 playback: test actual MP3 file playback over WiFi.
void test_audio_wifi_mp3_playback(void);

#define TAG "AudioTest"

// Start Test Suite
void setUp(void) {
    // Record memory state before each test
    setUpMemoryMonitoring();
}

void tearDown(void) {
    // Check for memory leaks after each test
    tearDownMemoryMonitoring("test");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Mount LittleFS for local file access
    ESP_LOGI(TAG, "Mounting LittleFS...");
    if (!LittleFS.begin(true, "/data", 5, "littlefs")) {
        ESP_LOGE(TAG, "LittleFS Mount Failed");
        ESP_LOGE(TAG, "MP3 playback tests will fail without LittleFS");
    } else {
        ESP_LOGI(TAG, "LittleFS mounted successfully");
    }

    // Print test suite memory summary
    printTestSuiteMemorySummary("Audio", true);
    
    UNITY_BEGIN();
    
    // Run all tests with memory monitoring and colorful output
    RUN_TEST_EX(TAG, test_audio_initialize_and_ready);
    RUN_TEST_EX(TAG, test_audio_tick_when_ready_is_safe);
    RUN_TEST_EX(TAG, test_audio_set_volume_smoke);
    RUN_TEST_EX(TAG, test_audio_playback_state);
    RUN_TEST_EX(TAG, test_audio_stop_functionality);
    RUN_TEST_EX(TAG, test_audio_mp3_playback);
    RUN_TEST_EX(TAG, test_audio_wifi_mp3_playback);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("Audio", false);
}

void loop() {}