#include "main_screen.h"
#include "drivers/lvgl_drive.h"
#include "ui/ui.h"
#include "core/log.h"
#include "core/connection_monitor.h"
#include "core/runtime_state_manager.h"
#include "lvgl.h"

static const char *TAG = "MainScreen";

// Static instance for callbacks
MainScreen* MainScreen::instance_ = nullptr;

MainScreen::MainScreen(ServiceManager& serviceManager)
    : serviceManager_(serviceManager), initialized_(false) {
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
    
    if (!dictionaryService_.initialize()) {
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
    dictionaryService_.shutdown();
    initialized_ = false;
}

void MainScreen::enterMainState() {
    ESP_LOGI(TAG, "Entering main state");
    
    // Load the main screen
    loadScreen(ui_Main);
    
    // Set up UI elements
    addObjectToDefaultGroup(ui_InputWord);
    lv_textarea_set_text(ui_InputWord, "");
    lv_group_focus_obj(ui_InputWord);
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_TxtWord, "");
    lv_label_set_text(ui_TxtExplanation, "");
    lv_label_set_text(ui_TxtSampleSentence, "");
    
    // Initially hide the playing icon
    lv_obj_add_flag(ui_IcoPlaying, LV_OBJ_FLAG_HIDDEN);
    
    // Set up callbacks using the new KeyProcessor
    serviceManager_.keyProcessor().setSubmitCallback([]() {
        if (instance_) {
            String word = lv_textarea_get_text(ui_InputWord);
            if (word.length() > 0) {
                instance_->onWordSubmitted(word);
            }
        }
    });
    
    serviceManager_.keyProcessor().setKeyInCallback([](char key) {
        if (instance_) {
            instance_->onKeyPressed(key);
        }
    });
    
    // Update connection status
    updateConnectionStatus();
    
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
    currentResult_ = dictionaryService_.lookupWord(word);
    
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
        dictionaryService_.playAudio(currentWord_, "word");
    }
}

void MainScreen::onPlayExplanationAudio() {
    if (currentWord_.length() > 0) {
        dictionaryService_.playAudio(currentWord_, "explanation");
    }
}

void MainScreen::onPlaySampleAudio() {
    if (currentWord_.length() > 0) {
        dictionaryService_.playAudio(currentWord_, "sample");
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
    // Subscribe to function key events
    serviceManager_.keyProcessor().onFunctionKeyEvent(
        [this](const FunctionKeyEvent& event) {
            onFunctionKeyEvent(event);
        }
    );
    
    // Subscribe to audio events
    EventSystem::instance().getEventBus<AudioEvent>().subscribe(
        [this](const AudioEvent& event) {
            onAudioEvent(event);
        }
    );
}

void MainScreen::onFunctionKeyEvent(const FunctionKeyEvent& event) {
    // Only handle audio events in main state
    if (serviceManager_.isSystemReady()) {
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
    }
}

void MainScreen::onAudioEvent(const AudioEvent& event) {
    switch (event.type) {
        case AudioEvent::PlaybackStarted:
            ESP_LOGI(TAG, "Audio playback started - showing playing icon");
            lv_obj_remove_flag(ui_IcoPlaying, LV_OBJ_FLAG_HIDDEN);
            break;
            
        case AudioEvent::PlaybackStopped:
        case AudioEvent::PlaybackCompleted:
        case AudioEvent::PlaybackError:
            ESP_LOGI(TAG, "Audio playback stopped - hiding playing icon");
            lv_obj_add_flag(ui_IcoPlaying, LV_OBJ_FLAG_HIDDEN);
            break;
            
        default:
            break;
    }
}

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

void MainScreen::updateConnectionStatus() {
    // This would update UI indicators for connection status
    // For now, we'll just log the status
    bool wifiHealthy = ConnectionMonitor::instance().isWiFiHealthy();
    bool bleHealthy = ConnectionMonitor::instance().isBLEHealthy();
    
    ESP_LOGI(TAG, "Connection status - WiFi: %s, BLE: %s", 
             wifiHealthy ? "Connected" : "Disconnected",
             bleHealthy ? "Connected" : "Disconnected");
    
    // TODO: Update UI indicators (status lights, buttons, etc.)
    // This would involve updating LVGL objects to show connection status
}

void MainScreen::onRequestWiFiRecovery() {
    ESP_LOGI(TAG, "User requested WiFi recovery");
    RuntimeStateManager::instance().requestWiFiSettings();
}

void MainScreen::onRequestBLERecovery() {
    ESP_LOGI(TAG, "User requested BLE recovery");
    RuntimeStateManager::instance().requestKeyboardSettings();
}
