#include <Arduino.h>
#include <unity.h>
#include "audio_manager.h"
#include "memory_test_helper.h"
#include "LittleFS.h"
#include <WiFi.h>
#include "test_wifi_credentials.h"

using namespace dict;

// =================================== TESTS ===================================

void test_audio_initialize_and_ready(void) {
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(audio.isReady());
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
}

void test_audio_tick_when_ready_is_safe(void) {
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Test that tick() can be called safely when ready
    audio.tick();
    audio.tick();
    audio.tick();
    
    // Verify audio is still ready after ticks
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
}

void test_audio_set_volume_smoke(void) {
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Test volume setting
    audio.setVolume(0.0f);
    audio.setVolume(0.5f);
    audio.setVolume(1.0f);
    audio.setVolume(0.7f);
    
    // Verify audio is still ready after volume changes
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
}

void test_audio_playback_state(void) {
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Test initial playback state
    TEST_ASSERT_FALSE(audio.isCurrentlyPlaying());
    TEST_ASSERT_FALSE(audio.isCurrentlyPaused());
    
    // Test that we can call these methods multiple times
    TEST_ASSERT_FALSE(audio.isCurrentlyPlaying());
    TEST_ASSERT_FALSE(audio.isCurrentlyPaused());
    
    // Verify audio is still ready
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
}

void test_audio_stop_functionality(void) {
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Test stop() when not playing (should not crash)
    TEST_ASSERT_FALSE(audio.stop());
    
    // Test stop() multiple times (should not crash)
    TEST_ASSERT_FALSE(audio.stop());
    TEST_ASSERT_FALSE(audio.stop());
    
    // Verify audio is still ready after stop calls
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Verify playback state is still correct
    TEST_ASSERT_FALSE(audio.isCurrentlyPlaying());
    TEST_ASSERT_FALSE(audio.isCurrentlyPaused());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
}

void test_audio_mp3_playback(void) {
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Test playing with local MP3 file from LittleFS
    const char* localMp3File = "/apple.mp3";
    
    ESP_LOGI("AudioTest", "[MP3 Playback Test] Attempting to play local file: %s", localMp3File);
    
    // Check if file exists first
    if (!LittleFS.exists(localMp3File)) {
        ESP_LOGW("AudioTest", "[MP3 Playback Test] File %s does not exist in LittleFS", localMp3File);
        ESP_LOGW("AudioTest", "[MP3 Playback Test] Skipping playback test - file not found");
        // Still test that play() doesn't crash with non-existent file
        bool playResult = audio.play(localMp3File);
        ESP_LOGI("AudioTest", "[MP3 Playback Test] Play result (file not found): %s", playResult ? "SUCCESS" : "FAILED");
        TEST_ASSERT_FALSE_MESSAGE(playResult, "Play should fail for non-existent file");
    } else {
        ESP_LOGI("AudioTest", "[MP3 Playback Test] File %s exists, attempting playback", localMp3File);
        // This should succeed if the file exists in LittleFS
        audio.setVolume(0.7f);
        bool playResult = audio.play(localMp3File);
        ESP_LOGI("AudioTest", "[MP3 Playback Test] Play result: %s", playResult ? "SUCCESS" : "FAILED");
    }
    
    // Test playback state after play attempt
    ESP_LOGI("AudioTest", "[MP3 Playback Test] isCurrentlyPlaying: %s", audio.isCurrentlyPlaying() ? "true" : "false");
    ESP_LOGI("AudioTest", "[MP3 Playback Test] isCurrentlyPaused: %s", audio.isCurrentlyPaused() ? "true" : "false");
    
    delay(1000); // wait for the sound (another thread) to play, but only for 1 second.
    
    // Test stop functionality
    ESP_LOGI("AudioTest", "[MP3 Playback Test] Testing stop...");
    bool stopResult = audio.stop();
    ESP_LOGI("AudioTest", "[MP3 Playback Test] Stop result: %s", stopResult ? "SUCCESS" : "FAILED");
    
    // Verify state after stop
    TEST_ASSERT_FALSE(audio.isCurrentlyPlaying());
    TEST_ASSERT_FALSE(audio.isCurrentlyPaused());
    
    // Verify audio is still ready
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
}

void test_audio_wifi_mp3_playback(void) {
    const char* TAG = "AudioTest";
    const char* wifiSSID = TEST_WIFI_SSID;
    const char* wifiPassword = TEST_WIFI_PASSWORD;
    const char* mp3Url = "http://192.168.1.164:1234/apple.mp3";
    
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Starting WiFi MP3 playback test");
    
    // Initialize AudioManager
    AudioManager audio;
    TEST_ASSERT_TRUE_MESSAGE(audio.initialize(), "Audio initialize() failed");
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Connect to WiFi
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Connecting to WiFi: %s", wifiSSID);
    WiFi.begin(wifiSSID, wifiPassword);
    
    // Wait for WiFi connection (with timeout)
    int wifiTimeout = 0;
    while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
        delay(500);
        wifiTimeout++;
        ESP_LOGI(TAG, "[WiFi MP3 Playback Test] WiFi connecting... (%d/20)", wifiTimeout);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGE(TAG, "[WiFi MP3 Playback Test] WiFi connection failed - skipping test");
        TEST_FAIL_MESSAGE("WiFi connection failed");
        audio.shutdown();
        return;
    }
    
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] WiFi connected! IP: %s", WiFi.localIP().toString().c_str());
    
    // Test URL playback
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Attempting to play URL: %s", mp3Url);
    
    bool playResult = audio.play(mp3Url);
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Play result: %s", playResult ? "SUCCESS" : "FAILED");
    
    if (playResult) {
        // Wait a bit for playback to start
        delay(1000);
        
        // Verify playback state
        TEST_ASSERT_TRUE(audio.isCurrentlyPlaying());
        TEST_ASSERT_FALSE(audio.isCurrentlyPaused());
        
        ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Playback started successfully");
        
        // Let it play for a few seconds
        ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Letting audio play for 2 seconds...");
        delay(2000);
        
        // Stop playback
        ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Stopping playback");
        bool stopResult = audio.stop();
        TEST_ASSERT_TRUE_MESSAGE(stopResult, "Audio stop() failed");
        
        // Verify state after stop
        TEST_ASSERT_FALSE(audio.isCurrentlyPlaying());
        TEST_ASSERT_FALSE(audio.isCurrentlyPaused());
        
        ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Playback stopped successfully");
    } else {
        ESP_LOGE(TAG, "[WiFi MP3 Playback Test] Failed to start playback");
        TEST_FAIL_MESSAGE("Failed to start URL playback");
    }
    
    // Disconnect WiFi
    WiFi.disconnect();
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] WiFi disconnected");
    
    // Verify audio is still ready
    TEST_ASSERT_TRUE(audio.isReady());
    
    // Explicitly shutdown AudioManager
    audio.shutdown();
    
    ESP_LOGI(TAG, "[WiFi MP3 Playback Test] Test completed");
}