#include "main_screen_controller.h"
#include "drivers/lvgl_drive.h"
#include "ui/ui.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "MainScreenController";

// Static instance for callbacks
MainScreenController* MainScreenController::instance_ = nullptr;

MainScreenController::MainScreenController(Services& services)
    : services_(services), initialized_(false) {
    instance_ = this;
}

MainScreenController::~MainScreenController() {
    shutdown();
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool MainScreenController::initialize() {
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

void MainScreenController::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down main screen controller...");
    dictionaryService_.shutdown();
    initialized_ = false;
}

void MainScreenController::enterMainState() {
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
    
    // Set up callbacks using the new KeyProcessor
    Services::instance().keyProcessor().setSubmitCallback([]() {
        if (instance_) {
            String word = lv_textarea_get_text(ui_InputWord);
            if (word.length() > 0) {
                instance_->onWordSubmitted(word);
            }
        }
    });
    
    Services::instance().keyProcessor().setKeyInCallback([](char key) {
        if (instance_) {
            instance_->onKeyPressed(key);
        }
    });
    
    // Clear current state
    currentWord_ = "";
    currentResult_ = DictionaryResult();
}

void MainScreenController::onWordSubmitted(const String& word) {
    if (word.length() == 0) {
        return;
    }
    
    ESP_LOGI(TAG, "Word submitted: %s", word.c_str());
    
    // Switch to result mode
    showResultMode();
    lv_label_set_text(ui_TxtWord, word.c_str());
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

void MainScreenController::onKeyPressed(char key) {
    ESP_LOGD(TAG, "Key pressed: %c", key);
    showInputMode();
}

void MainScreenController::onPlayWordAudio() {
    if (currentWord_.length() > 0) {
        dictionaryService_.playAudio(currentWord_, "word");
    }
}

void MainScreenController::onPlayExplanationAudio() {
    if (currentWord_.length() > 0) {
        dictionaryService_.playAudio(currentWord_, "explanation");
    }
}

void MainScreenController::onPlaySampleAudio() {
    if (currentWord_.length() > 0) {
        dictionaryService_.playAudio(currentWord_, "sample_sentence");
    }
}

void MainScreenController::updateUI() {
    // Update UI based on current state
    // This can be called periodically to refresh the display
}

String MainScreenController::getCurrentWord() const {
    return currentWord_;
}

void MainScreenController::setupEventSubscriptions() {
    // Subscribe to function key events
    services_.keyProcessor().onFunctionKeyEvent(
        [this](const FunctionKeyEvent& event) {
            onFunctionKeyEvent(event);
        }
    );
}

void MainScreenController::onFunctionKeyEvent(const FunctionKeyEvent& event) {
    // Only handle audio events in main state
    if (services_.isSystemReady()) {
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

void MainScreenController::showInputMode() {
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_group_focus_obj(ui_InputWord);
}

void MainScreenController::showResultMode() {
    lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
}

void MainScreenController::clearResults() {
    lv_label_set_text(ui_TxtWord, "");
    lv_label_set_text(ui_TxtExplanation, "");
    lv_label_set_text(ui_TxtSampleSentence, "");
    currentWord_ = "";
    currentResult_ = DictionaryResult();
}


// Static callback implementations
void MainScreenController::submitFormCallback() {
    if (instance_) {
        String word = lv_textarea_get_text(ui_InputWord);
        instance_->onWordSubmitted(word);
        lv_textarea_set_text(ui_InputWord, "");
    }
}

void MainScreenController::keyInCallback(char key) {
    if (instance_) {
        instance_->onKeyPressed(key);
    }
}
