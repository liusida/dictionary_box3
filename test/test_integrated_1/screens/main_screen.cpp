#include "main_screen.h"
#include "drivers_audio/audio_manager.h"
#include "drivers_display/lvgl_helper.h"
#include "ui/ui.h"

namespace dict {

static const char *TAG = "MainScreen";

extern AudioManager *g_audio;

MainScreen::MainScreen() : initialized_(false), visible_(false) {}

MainScreen::~MainScreen() { shutdown(); }

bool MainScreen::initialize() {
    if (initialized_) {
        return true;
    }

    dictionaryApi_.initialize();

    initialized_ = true;
    return true;
}

void MainScreen::shutdown() {
    if (!initialized_) {
        return;
    }

    dictionaryApi_.shutdown();

    initialized_ = false;
    visible_ = false;
}

void MainScreen::tick() {
    if (!initialized_) {
        return;
    }
}

bool MainScreen::isReady() const { return initialized_; }

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
    lvglSetKeyCallbacks(
        [this]() {
            // Submit callback - called when Enter is pressed
            ESP_LOGI(TAG, "Submit pressed");
            onSubmit();
        },
        [this](char key) {
            // Key input callback - called for each character
            ESP_LOGD(TAG, "Key input: %c", key);
            onKeyIn(key);
        });
    lvglSetFunctionKeyCallbacks([this](const FunctionKeyEvent &event) {
        // Function key input callback - called for each function key
        ESP_LOGD(TAG, "Function key input: %d", event.type);
        onFunctionKeyEvent(event);
    });
    ESP_LOGI(TAG, "LVGL key event handler installed for MainScreen");

    visible_ = true;
}

void MainScreen::hide() {
    if (!initialized_) {
        return;
    }

    // Remove LVGL key event handler when screen is hidden
    lvglSetKeyCallbacks(nullptr, nullptr);
    lvglSetFunctionKeyCallbacks(nullptr);
    ESP_LOGI(TAG, "LVGL key event handler removed from MainScreen");

    visible_ = false;
}

bool MainScreen::isVisible() const { return visible_; }

void MainScreen::onSubmit() {
    currentWord_ = lv_textarea_get_text(ui_InputWord);
    if (currentWord_.length() == 0) {
        return;
    }
    ESP_LOGI(TAG, "Submit action triggered");
    lv_textarea_set_text(ui_InputWord, "");
    lv_label_set_text(ui_TxtWord, currentWord_.c_str());
    lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    currentResult_ = dictionaryApi_.lookupWord(currentWord_);
    if (currentResult_.success) {
        currentWord_ = currentResult_.word;
        lv_label_set_text(ui_TxtWord, currentResult_.word.c_str());
        lv_label_set_text(ui_TxtExplanation, currentResult_.explanation.c_str());
        lv_label_set_text(ui_TxtSampleSentence, currentResult_.sampleSentence.c_str());
    } else {
        lv_label_set_text(ui_TxtExplanation, "Request failed!");
    }
}

void MainScreen::onKeyIn(char key) {
    ESP_LOGD(TAG, "Key in: %c", key);
    if (lv_obj_has_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(ui_InputWord);
        const char str[2] = {key, '\0'};
        lv_textarea_set_text(ui_InputWord, str);
    }
    // Most of the keyins are handled by handleKeyEvent to LVGL's default group
}

void MainScreen::onFunctionKeyEvent(const FunctionKeyEvent &event) {
    if (currentWord_.length() == 0) {
        return;
    }
    ESP_LOGD(TAG, "Function key input: %d", event.type);
    AudioUrl audioUrl;
    switch (event.type) {
    case FunctionKeyEvent::ReadWord:
        audioUrl = dictionaryApi_.getAudioUrl(currentWord_, "word");
        break;
    case FunctionKeyEvent::ReadExplanation:
        audioUrl = dictionaryApi_.getAudioUrl(currentWord_, "explanation");
        break;
    case FunctionKeyEvent::ReadSampleSentence:
        audioUrl = dictionaryApi_.getAudioUrl(currentWord_, "sample");
        break;
    default:
        break;
    }
    ESP_LOGI(TAG, "Playing audio: %s", audioUrl.url.c_str());
    if (g_audio) {
        g_audio->play(audioUrl.url.c_str());
    }
}

void MainScreen::onConnectionReady() {
    if (dictionaryApi_.isReady()) {
        dictionaryApi_.prewarm();
    }
}

} // namespace dict