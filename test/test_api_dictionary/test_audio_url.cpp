#include <Arduino.h>
#include <unity.h>
#include "../../lib/api_dictionary/dictionary_api.h"

using namespace dict;

// =================================== TESTS ===================================

void test_dictionary_api_audio_url_configuration(void) {
    DictionaryApi api;
    
    // Test default audio URL
    String defaultAudioUrl = api.getAudioBaseUrl();
    TEST_ASSERT_EQUAL_STRING("https://dict.liusida.com/api/audio/stream", defaultAudioUrl.c_str());
    
    // Test setting new audio URL
    String newAudioUrl = "https://example.com/audio";
    api.setAudioBaseUrl(newAudioUrl);
    TEST_ASSERT_EQUAL_STRING(newAudioUrl.c_str(), api.getAudioBaseUrl().c_str());
    
    // Test setting empty audio URL
    api.setAudioBaseUrl("");
    TEST_ASSERT_EQUAL_STRING("", api.getAudioBaseUrl().c_str());
}

void test_dictionary_api_audio_url_generation(void) {
    DictionaryApi api;
    
    api.initialize();
    
    // Test audio URL generation - should work with WiFi connection
    AudioUrl audioUrl = api.getAudioUrl("test", "word");
    TEST_ASSERT_TRUE_MESSAGE(audioUrl.valid, "Audio URL should be valid with WiFi connection");
    
    // Test with invalid word
    AudioUrl invalidUrl = api.getAudioUrl("", "word");
    TEST_ASSERT_FALSE(invalidUrl.valid);
    
    // Test with null word
    AudioUrl nullUrl = api.getAudioUrl("null", "word");
    TEST_ASSERT_FALSE(nullUrl.valid);
}

void test_dictionary_api_audio_url_types(void) {
    DictionaryApi api;
    
    api.initialize();
    
    // Test different audio types - should work with WiFi connection
    AudioUrl wordUrl = api.getAudioUrl("test", "word");
    AudioUrl explanationUrl = api.getAudioUrl("test", "explanation");
    AudioUrl sampleUrl = api.getAudioUrl("test", "sample_sentence");
    
    // All should be valid with WiFi connection
    TEST_ASSERT_TRUE(wordUrl.valid);
    TEST_ASSERT_TRUE(explanationUrl.valid);
    TEST_ASSERT_TRUE(sampleUrl.valid);
    
    // Verify audio types are preserved
    TEST_ASSERT_EQUAL_STRING("word", wordUrl.audioType.c_str());
    TEST_ASSERT_EQUAL_STRING("explanation", explanationUrl.audioType.c_str());
    TEST_ASSERT_EQUAL_STRING("sample_sentence", sampleUrl.audioType.c_str());
}

void test_dictionary_api_audio_url_error_handling(void) {
    DictionaryApi api;
    
    api.initialize();
    
    // Test audio URL error handling
    AudioUrl audioUrl1 = api.getAudioUrl("", "word");
    TEST_ASSERT_FALSE(audioUrl1.valid);
    
    AudioUrl audioUrl2 = api.getAudioUrl("null", "word");
    TEST_ASSERT_FALSE(audioUrl2.valid);
    
    // Test with invalid audio type
    AudioUrl audioUrl3 = api.getAudioUrl("test", "");
    TEST_ASSERT_FALSE(audioUrl3.valid);
}
