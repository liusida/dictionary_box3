#include "main_screen.h"
#include "drivers/lvgl_drive.h"
#include "network_control.h"
#include "audio_manager.h"
#include "drivers/ble_keyboard.h"
#include "ui/ui.h"
#include "core/log.h"
#include "lvgl.h"

static const char *TAG = "MainScreen";

// Static instance for callbacks
MainScreen* MainScreen::instance_ = nullptr;

MainScreen::MainScreen(NetworkControl& wifiDriver, AudioManager& audioDriver, BLEKeyboard& bleDriver)
    : wifiDriver_(wifiDriver), audioDriver_(audioDriver), bleDriver_(bleDriver), 
      dictionaryApi_(audioDriver), initialized_(false) {
    instance_ = this;
}

MainScreen::~MainScreen() {
    shutdown();
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool MainScreen::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing main screen controller...");
    
    if (!dictionaryApi_.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize dictionary service");
        return false;
    }
    
    setupEventSubscriptions();
    initialized_ = true;
    
    ESP_LOGI(TAG, "Main screen controller initialized");
    return true;
}

void MainScreen::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down main screen controller...");
    dictionaryApi_.shutdown();
    initialized_ = false;
}

void MainScreen::tick() {
    // Update status icons based on driver state (following simplified architecture)
    updateStatusIcons();
}

void MainScreen::updateStatusIcons() {
    // Update WiFi status icon based on driver state
    if (wifiDriver_.isConnected()) {
        // TODO: Set WiFi icon to green/connected state
        ESP_LOGD(TAG, "WiFi connected - updating status icon");
    } else {
        // TODO: Set WiFi icon to red/disconnected state
        ESP_LOGD(TAG, "WiFi disconnected - updating status icon");
    }
    
    // Update BLE status icon based on driver state
    if (bleDriver_.isConnected()) {
        // TODO: Set BLE icon to blue/connected state
        ESP_LOGD(TAG, "BLE connected - updating status icon");
    } else {
        // TODO: Set BLE icon to red/disconnected state
        ESP_LOGD(TAG, "BLE disconnected - updating status icon");
    }
    
    // Update audio status icon based on driver state
    // TODO: Check audio driver state and update icon accordingly
}

void MainScreen::showMainScreen() {
    ESP_LOGI(TAG, "Showing main screen");
    
    // Load the main screen
    lv_disp_load_scr(ui_Main);
    
    // Set up UI elements first
    addObjectToDefaultGroup(ui_InputWord);
    lv_textarea_set_text(ui_InputWord, "");
    lv_group_focus_obj(ui_InputWord);
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_TxtWord, "");
    lv_label_set_text(ui_TxtExplanation, "");
    lv_label_set_text(ui_TxtSampleSentence, "");
    
    // Set up callbacks directly (following simplified architecture)
    // TODO: Set up submit callback through BLE driver or direct UI callback
    
    // TODO: Set up key input callback through BLE driver
    
    // Clear current state
    currentWord_ = "";
    currentResult_ = DictionaryResult();
}

void MainScreen::onWordSubmitted(const String& word) {
    if (word.length() == 0) {
        return;
    }
    
    ESP_LOGI(TAG, "Word submitted: %s", word.c_str());
    
    // Switch to result mode
    showResultMode();
    lv_label_set_text(ui_TxtWord, word.c_str());
    lv_textarea_set_text(ui_InputWord, "");
    currentWord_ = word;
    
    // Look up the word
    currentResult_ = dictionaryApi_.lookupWord(word);
    
    if (currentResult_.success) {
        lv_label_set_text(ui_TxtWord, currentResult_.word.c_str());
        lv_label_set_text(ui_TxtExplanation, currentResult_.explanation.c_str());
        lv_label_set_text(ui_TxtSampleSentence, currentResult_.sampleSentence.c_str());
    } else {
        lv_label_set_text(ui_TxtExplanation, "Request failed");
    }
}

void MainScreen::onKeyPressed(char key) {
    ESP_LOGD(TAG, "Key pressed: %c", key);
    showInputMode();
}

void MainScreen::onPlayWordAudio() {
    if (currentWord_.length() > 0) {
        // TODO: Update audio status icon manually
        dictionaryApi_.playAudio(currentWord_, "word");
    }
}

void MainScreen::onPlayExplanationAudio() {
    if (currentWord_.length() > 0) {
        // TODO: Update audio status icon manually
        dictionaryApi_.playAudio(currentWord_, "explanation");
    }
}

void MainScreen::onPlaySampleAudio() {
    if (currentWord_.length() > 0) {
        // TODO: Update audio status icon manually
        dictionaryApi_.playAudio(currentWord_, "sample");
    }
}

void MainScreen::updateUI() {
    // Update UI based on current state
    // This can be called periodically to refresh the display
}

String MainScreen::getCurrentWord() const {
    return currentWord_;
}

void MainScreen::setupEventSubscriptions() {
    // Subscribe to function key events - simplified direct communication
    // TODO: Set up function key event callback through BLE driver
    
    // Note: Audio and Dictionary events will be handled through direct method calls
    // instead of the complex event system, following the simplified architecture
}

void MainScreen::onFunctionKeyEvent(const FunctionKeyEvent& event) {
    // Only handle audio events in main state
    if (wifiDriver_.isConnected()) {
        // TODO: Show BLE spinning animation for any function key manually
        
        switch (event.type) {
            case FunctionKeyEvent::ReadWord:
                onPlayWordAudio();
                break;
            case FunctionKeyEvent::ReadExplanation:
                onPlayExplanationAudio();
                break;
            case FunctionKeyEvent::ReadSampleSentence:
                onPlaySampleAudio();
                break;
            default:
                break;
        }
        
        // TODO: Return BLE to stable state after a short delay manually
    }
}

// Audio and Dictionary events are now handled through direct method calls
// following the simplified architecture principle of "Direct Communication"

void MainScreen::showInputMode() {
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_group_focus_obj(ui_InputWord);
}

void MainScreen::showResultMode() {
    lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
}

void MainScreen::clearResults() {
    lv_label_set_text(ui_TxtWord, "");
    lv_label_set_text(ui_TxtExplanation, "");
    lv_label_set_text(ui_TxtSampleSentence, "");
    currentWord_ = "";
    currentResult_ = DictionaryResult();
}

// Static callback implementations
void MainScreen::submitFormCallback() {
    if (instance_) {
        String word = lv_textarea_get_text(ui_InputWord);
        instance_->onWordSubmitted(word);
        lv_textarea_set_text(ui_InputWord, "");
    }
}

void MainScreen::keyInCallback(char key) {
    if (instance_) {
        instance_->onKeyPressed(key);
    }
}
