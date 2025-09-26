#include "main_screen.h"
#include "ui/ui.h"
#include "drivers_display/lvgl_helper.h"

namespace dict {

static const char *TAG = "MainScreen";

MainScreen::MainScreen()
    : initialized_(false), visible_(false) {
}

MainScreen::~MainScreen() {
    shutdown();
}

bool MainScreen::initialize() {
    if (initialized_) {
        return true;
    }
    
    // TODO: Initialize main screen components
    
    initialized_ = true;
    return true;
}

void MainScreen::shutdown() {
    if (!initialized_) {
        return;
    }
    
    // TODO: Clean up main screen resources
    
    initialized_ = false;
    visible_ = false;
}

void MainScreen::tick() {
    if (!initialized_) {
        return;
    }
    
    // TODO: Process main screen updates
}

bool MainScreen::isReady() const {
    return initialized_;
}

void MainScreen::show() {
    if (!initialized_) {
        return;
    }
    
    // Initialize and show the main UI screen
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

    // Install LVGL key event handler when screen becomes active
    lvglInstallKeyEventHandler(
        [this]() {
            // Submit callback - called when Enter is pressed
            ESP_LOGI(TAG, "Submit pressed");
            onSubmit();
        },
        [this](char key) {
            // Key input callback - called for each character
            ESP_LOGD(TAG, "Key input: %c", key);
            onKeyIn(key);
        }
    );
    ESP_LOGI(TAG, "LVGL key event handler installed for MainScreen");

    visible_ = true;
}

void MainScreen::hide() {
    if (!initialized_) {
        return;
    }
    
    // Remove LVGL key event handler when screen is hidden
    lvglRemoveKeyEventHandler();
    ESP_LOGI(TAG, "LVGL key event handler removed from MainScreen");
    
    visible_ = false;
}

bool MainScreen::isVisible() const {
    return visible_;
}

void MainScreen::onSubmit() {
    ESP_LOGI(TAG, "Submit action triggered");
    currentWord_ = lv_textarea_get_text(ui_InputWord);
    lv_textarea_set_text(ui_InputWord, "");
    lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    currentResult_ = dictionaryApi_.lookupWord(currentWord_);
    if (currentResult_.success) {
      lv_label_set_text(ui_TxtWord, currentResult_.word.c_str());
      lv_label_set_text(ui_TxtExplanation, currentResult_.explanation.c_str());
      lv_label_set_text(ui_TxtSampleSentence, currentResult_.sampleSentence.c_str());
  } else {
      lv_label_set_text(ui_TxtExplanation, "Request failed!");
  }
}

void MainScreen::onKeyIn(char key) {
    ESP_LOGD(TAG, "Key in: %c", key);
    // TODO: Handle key input (e.g., update UI, process input)
}


} // namespace dict