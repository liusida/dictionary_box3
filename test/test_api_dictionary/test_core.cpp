#include <Arduino.h>
#include <unity.h>
#include "../../lib/api_dictionary/dictionary_api.h"

using namespace dict;

// =================================== TESTS ===================================

void test_dictionary_api_constructor_destructor(void) {
    DictionaryApi* api = new DictionaryApi();
    TEST_ASSERT_NOT_NULL(api);
    TEST_ASSERT_FALSE(api->isReady());
    delete api;
}

void test_dictionary_api_initialize_and_ready(void) {
    DictionaryApi api;
    
    TEST_ASSERT_FALSE(api.isReady());
    
    bool result = api.initialize();
    TEST_ASSERT_TRUE_MESSAGE(result, "DictionaryApi initialize() failed");
}

void test_dictionary_api_shutdown(void) {
    DictionaryApi api;
    
    api.initialize();
    api.shutdown();
    
    // After shutdown, should not be ready
    TEST_ASSERT_FALSE(api.isReady());
}

void test_dictionary_api_url_encoding(void) {
    DictionaryApi api;
    
    // Test basic encoding
    String result = api.urlEncode("hello");
    TEST_ASSERT_EQUAL_STRING("hello", result.c_str());
    
    // Test space encoding
    result = api.urlEncode("hello world");
    TEST_ASSERT_EQUAL_STRING("hello%20world", result.c_str());
    
    // Test special characters
    result = api.urlEncode("hello@world#test");
    TEST_ASSERT_EQUAL_STRING("hello%40world%23test", result.c_str());
    
    // Test empty string
    result = api.urlEncode("");
    TEST_ASSERT_EQUAL_STRING("", result.c_str());
    
    // Test unicode characters
    result = api.urlEncode("café");
    TEST_ASSERT_TRUE(result.length() > 4); // Should be URL encoded
}

void test_dictionary_api_word_validation(void) {
    DictionaryApi api;
    
    // Test valid words
    TEST_ASSERT_TRUE(api.isWordValid("hello"));
    TEST_ASSERT_TRUE(api.isWordValid("test"));
    TEST_ASSERT_TRUE(api.isWordValid("word123"));
    
    // Test invalid words
    TEST_ASSERT_FALSE(api.isWordValid(""));
    TEST_ASSERT_FALSE(api.isWordValid("null"));
    TEST_ASSERT_FALSE(api.isWordValid("NULL"));
    TEST_ASSERT_FALSE(api.isWordValid("Null"));
}

void test_dictionary_api_configuration(void) {
    DictionaryApi api;
    
    // Test default URL
    String defaultUrl = api.getBaseUrl();
    TEST_ASSERT_EQUAL_STRING("https://dict.liusida.com/api/define", defaultUrl.c_str());
    
    // Test setting new URL
    String newUrl = "https://example.com/api";
    api.setBaseUrl(newUrl);
    TEST_ASSERT_EQUAL_STRING(newUrl.c_str(), api.getBaseUrl().c_str());
    
    // Test setting empty URL
    api.setBaseUrl("");
    TEST_ASSERT_EQUAL_STRING("", api.getBaseUrl().c_str());
}

void test_dictionary_api_ready_state(void) {
    DictionaryApi api;
    
    // Not ready before initialization
    TEST_ASSERT_FALSE(api.isReady());
    
    // Ready after initialization (assuming WiFi is connected in setup)
    api.initialize();
    TEST_ASSERT_TRUE_MESSAGE(api.isReady(), "Should be ready after initialization with WiFi connection");
}

void test_dictionary_api_error_handling(void) {
    DictionaryApi api;
    
    api.initialize();
    
    // Test error handling with invalid inputs
    DictionaryResult result1 = api.lookupWord("");
    TEST_ASSERT_FALSE(result1.success);
    TEST_ASSERT_EQUAL_STRING("", result1.word.c_str());
    
    DictionaryResult result2 = api.lookupWord("null");
    TEST_ASSERT_FALSE(result2.success);
    TEST_ASSERT_EQUAL_STRING("", result2.word.c_str());
}
