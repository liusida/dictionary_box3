#pragma once
#include <string>
#include <Arduino.h>

// Undefine Arduino's word macro to avoid conflicts
#ifdef word
#undef word
#endif

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
 * @brief Service for dictionary operations and audio playback
 * 
 * Handles all communication with the dictionary API and audio streaming.
 * Extracted from main_screen_control.cpp to separate concerns.
 */
class DictionaryService {
public:
    DictionaryService();
    ~DictionaryService();
    
    /**
     * @brief Initialize the service
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Shutdown the service
     */
    void shutdown();
    
    /**
     * @brief Look up a word in the dictionary
     * @param word The word to look up
     * @return DictionaryResult with the lookup results
     */
    DictionaryResult lookupWord(const String& word);
    
    /**
     * @brief Play audio for a word
     * @param word The word to play audio for
     * @param audioType Type of audio ("word", "explanation", "sample_sentence")
     * @return true if audio playback started successfully
     */
    bool playAudio(const String& word, const String& audioType);
    
    /**
     * @brief Check if the service is ready (WiFi connected)
     * @return true if ready, false otherwise
     */
    bool isReady() const;
    
    /**
     * @brief Set the base URL for the dictionary API
     * @param url The base URL
     */
    void setBaseUrl(const String& url);
    
    /**
     * @brief Get the current base URL
     * @return The current base URL
     */
    String getBaseUrl() const;
    
private:
    String baseUrl_;
    String audioUrl_;
    bool initialized_;
    
    // Helper methods
    String urlEncode(const String& str);
    bool isWordValid(const String& word);
};
