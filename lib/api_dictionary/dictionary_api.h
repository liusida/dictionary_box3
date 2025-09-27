#pragma once
#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace dict {

/**
 * @brief Result structure for dictionary lookups
 */
struct DictionaryResult {
    String word;
    String explanation;
    String sampleSentence;
    bool success = false;
    
    DictionaryResult() = default;
    DictionaryResult(const String& w, const String& e, const String& s, bool s_ok)
        : word(w), explanation(e), sampleSentence(s), success(s_ok) {}
};

/**
 * @brief Audio URL structure for audio playback
 */
struct AudioUrl {
    String url;
    String audioType;
    bool valid = false;
    
    AudioUrl() = default;
    AudioUrl(const String& u, const String& type, bool v = true)
        : url(u), audioType(type), valid(v) {}
};

/**
 * @brief Pure dictionary API client for word lookups
 * 
 * Handles communication with the dictionary API for word lookups.
 * Provides audio URL generation for external audio playback.
 * Decoupled from audio hardware - higher-level code handles audio playback.
 */
class DictionaryApi {
public:
    // Constructor/Destructor
    DictionaryApi();
    ~DictionaryApi();
    
    // Core lifecycle methods
    bool initialize(); // Initialize the API client
    void shutdown(); // Shutdown the API client
    bool isReady() const; // Check if the API client is ready (WiFi connected)
    
    // Main functionality methods
    DictionaryResult lookupWord(const String& word); // Look up a word in the dictionary
    AudioUrl getAudioUrl(const String& word, const String& audioType); // Get audio URL for a word
    void prewarm(); // Prewarm the API client
    
    // Configuration methods
    void setBaseUrl(const String& url); // Set the base URL for the dictionary API
    String getBaseUrl() const; // Get the current base URL
    void setAudioBaseUrl(const String& url); // Set the base URL for audio streaming
    String getAudioBaseUrl() const; // Get the current audio base URL
    
    // Helper methods (public for testing)
    String urlEncode(const String& str); // URL encode a string
    bool isWordValid(const String& word); // Validate word input
    
private:
    // Configuration
    String hostname_;
    String baseUrl_;
    String audioBaseUrl_;
    bool initialized_;
    
    // Async prewarm task
    TaskHandle_t prewarmTaskHandle_;
    
    // Static task function for async prewarm
    static void prewarmTask(void* parameter);
};

} // namespace dict