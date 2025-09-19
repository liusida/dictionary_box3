#pragma once
#include "core/services.h"
#include "services/dictionary_service.h"
#include "core/event_system.h"
#include "input/key_processor.h"
#include "lvgl.h"

// Forward declarations for UI elements
extern "C" {
    extern lv_obj_t * ui_Main;
    extern lv_obj_t * ui_TxtWord;
    extern lv_obj_t * ui_InputWord;
    extern lv_obj_t * ui_TxtExplanation;
    extern lv_obj_t * ui_TxtSampleSentence;
}

/**
 * @brief Controller for the main screen UI
 * 
 * Handles all UI logic for the main screen, including word lookup,
 * audio playback, and user input. Separated from business logic
 * for better maintainability.
 */
class MainScreenController {
public:
    MainScreenController(Services& services);
    ~MainScreenController();
    
    /**
     * @brief Initialize the controller
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Shutdown the controller
     */
    void shutdown();
    
    /**
     * @brief Enter the main state and set up UI
     */
    void enterMainState();
    
    /**
     * @brief Handle word submission from UI
     * @param word The word to look up
     */
    void onWordSubmitted(const String& word);
    
    /**
     * @brief Handle key input from keyboard
     * @param key The key that was pressed
     */
    void onKeyPressed(char key);
    
    /**
     * @brief Play audio for the current word
     */
    void onPlayWordAudio();
    
    /**
     * @brief Play audio for the current explanation
     */
    void onPlayExplanationAudio();
    
    /**
     * @brief Play audio for the current sample sentence
     */
    void onPlaySampleAudio();
    
    /**
     * @brief Update the UI with current state
     */
    void updateUI();
    
    /**
     * @brief Get the current word being displayed
     * @return The current word
     */
    String getCurrentWord() const;
    
private:
    Services& services_;
    DictionaryService dictionaryService_;
    String currentWord_;
    DictionaryResult currentResult_;
    bool initialized_;
    
    // Event subscriptions
    void setupEventSubscriptions();
    void onFunctionKeyEvent(const FunctionKeyEvent& event);
    
    // UI helpers
    void showInputMode();
    void showResultMode();
    void clearResults();
    
    // Static callbacks for LVGL
    static void submitFormCallback();
    static void keyInCallback(char key);
    static MainScreenController* instance_;
};
