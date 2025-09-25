#pragma once
#include "business/dictionary_api.h"
#include "core/events.h"
#include "lvgl.h"

// Forward declarations for drivers
class NetworkControl;
class AudioManager;
class BLEKeyboard;

// Forward declarations for UI elements
extern "C" {
    extern lv_obj_t * ui_Main;
    extern lv_obj_t * ui_TxtWord;
    extern lv_obj_t * ui_InputWord;
    extern lv_obj_t * ui_TxtExplanation;
    extern lv_obj_t * ui_TxtSampleSentence;
}

/**
 * @brief Simplified main screen manager
 * 
 * Handles all UI logic for the main screen, including word lookup,
 * audio playback, and user input. Simplified version of the original
 * main screen controller.
 */
class MainScreen {
public:
    MainScreen(NetworkControl& wifiDriver, AudioManager& audioDriver, BLEKeyboard& bleDriver);
    ~MainScreen();
    
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
    void showMainScreen();
    
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
    
    /**
     * @brief Tick method for regular updates
     */
    void tick();
    
    /**
     * @brief Update status icons based on driver state (following simplified architecture)
     */
    void updateStatusIcons();
    
private:
    // Drivers (following simplified architecture)
    NetworkControl& wifiDriver_;
    AudioManager& audioDriver_;
    BLEKeyboard& bleDriver_;
    
    DictionaryApi dictionaryApi_;
    String currentWord_;
    DictionaryResult currentResult_;
    bool initialized_;
    
    // Event subscriptions - simplified direct communication
    void setupEventSubscriptions();
    void onFunctionKeyEvent(const FunctionKeyEvent& event);
    
    // UI helpers
    void showInputMode();
    void showResultMode();
    void clearResults();
    
    // Static callbacks for LVGL
    static void submitFormCallback();
    static void keyInCallback(char key);
    static MainScreen* instance_;
};
