#include <Arduino.h>
#include <unity.h>
#include "../../lib/api_dictionary/dictionary_api.h"
#include "../../lib/core_misc/memory_test_helper.h"
#include "network_control.h"
#include "test_wifi_credentials.h"
#include <WiFi.h>

using namespace dict;

// =================================== TESTS ===================================

void test_dictionary_api_initialize_and_ready(void) {
    DictionaryApi* api = new DictionaryApi();
    
    // Test initialization
    TEST_ASSERT_TRUE_MESSAGE(api->initialize(), "DictionaryApi initialize() failed");
    
    // Test ready state (should be true with WiFi)
    TEST_ASSERT_TRUE_MESSAGE(api->isReady(), "DictionaryApi should be ready with WiFi");
    
    // Test shutdown
    api->shutdown();
    TEST_ASSERT_FALSE_MESSAGE(api->isReady(), "DictionaryApi should not be ready after shutdown");
    
    delete api;
}

void test_dictionary_api_lookup_word(void) {
    DictionaryApi* api = new DictionaryApi();
    TEST_ASSERT_TRUE_MESSAGE(api->initialize(), "DictionaryApi initialize() failed");
    
    // Test word validation
    TEST_ASSERT_TRUE_MESSAGE(api->isWordValid("hello"), "Valid word should pass validation");
    TEST_ASSERT_FALSE_MESSAGE(api->isWordValid(""), "Empty word should fail validation");
    TEST_ASSERT_FALSE_MESSAGE(api->isWordValid("   "), "Whitespace-only word should fail validation");
    
    // Test lookup with WiFi (should succeed)
    DictionaryResult result = api->lookupWord("test");
    TEST_ASSERT_TRUE_MESSAGE(result.success, "Lookup should succeed with WiFi");
    
    api->shutdown();
    delete api;
}

void test_dictionary_api_get_audio_url(void) {
    DictionaryApi* api = new DictionaryApi();
    TEST_ASSERT_TRUE_MESSAGE(api->initialize(), "DictionaryApi initialize() failed");
    
    // Test audio URL generation
    AudioUrl audioUrl = api->getAudioUrl("test", "word");
    TEST_ASSERT_TRUE_MESSAGE(audioUrl.valid, "Audio URL should be valid");
    TEST_ASSERT_TRUE_MESSAGE(audioUrl.url.length() > 0, "Audio URL should not be empty");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("word", audioUrl.audioType.c_str(), "Audio type should match");
    
    // Test different audio types
    AudioUrl explanationUrl = api->getAudioUrl("test", "explanation");
    TEST_ASSERT_TRUE_MESSAGE(explanationUrl.valid, "Explanation audio URL should be valid");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("explanation", explanationUrl.audioType.c_str(), "Explanation audio type should match");
    
    AudioUrl sampleUrl = api->getAudioUrl("test", "sample");
    TEST_ASSERT_TRUE_MESSAGE(sampleUrl.valid, "Sample audio URL should be valid");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("sample", sampleUrl.audioType.c_str(), "Sample audio type should match");
    
    api->shutdown();
    delete api;
}

void test_dictionary_api_prewarm(void) {
    DictionaryApi* api = new DictionaryApi();
    TEST_ASSERT_TRUE_MESSAGE(api->initialize(), "DictionaryApi initialize() failed");
    
    // Test prewarm (should work with WiFi)
    api->prewarm();
    
    // Wait for async prewarm task to complete
    int timeout = 8000; // 8 second timeout
    int waited = 0;
    while (api->isPrewarmRunning() && waited < timeout) {
        delay(100);
        waited += 100;
    }
    
    TEST_ASSERT_FALSE_MESSAGE(api->isPrewarmRunning(), "Prewarm task should have completed after timeout");

    // Test that prewarm maintains ready state
    TEST_ASSERT_TRUE_MESSAGE(api->isReady(), "Prewarm should maintain ready state with WiFi");
    
    api->shutdown();
    delete api;
}
