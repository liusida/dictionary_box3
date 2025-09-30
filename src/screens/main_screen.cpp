#include "main_screen.h"
#include "drivers_audio/audio_manager.h"
#include "drivers_display/lvgl_helper.h"
#include "ui.h"
#include "ui_status/ui_status.h"
#include "network_control.h"

namespace dict {

static const char *TAG = "MainScreen";

extern NetworkControl *g_network;
extern AudioManager *g_audio;
extern StatusOverlay *g_status;

MainScreen::MainScreen() : initialized_(false), visible_(false), isWifiSettings_(false), isScreenActive_(false) {}

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
  isWifiSettings_ = false;

  // Initialize and show the main UI screen
  lv_disp_load_scr(ui_Main);
  // Set up UI elements first
  addObjectToDefaultGroup(ui_InputWord);
  lv_textarea_set_text(ui_InputWord, "");
  lv_group_focus_obj(ui_InputWord);
  lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);

  lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Line, LV_OBJ_FLAG_HIDDEN);
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
  isScreenActive_ = true;
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
  isScreenActive_ = false;
}

bool MainScreen::isVisible() const { return visible_; }

void MainScreen::onSubmit() {
  if (!isScreenActive_ && isWifiSettings_) {
    wifiSettingsScreen_.onSubmit();
    return;
  }
  currentWord_ = lv_textarea_get_text(ui_InputWord);
  currentWord_.replace("\b", ""); // Remove backspace
  currentWord_.replace("\0", ""); // Remove null
  currentWord_.replace("\r", ""); // Remove carriage return
  currentWord_.replace("\n", ""); // Remove newline
  currentWord_.trim();
  if (currentWord_.isEmpty() || currentWord_.length() == 0) {
    lv_label_set_text(ui_TxtExplanation, "Please enter a word to start.");
    lv_obj_add_flag(ui_Line, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  ESP_LOGI(TAG, "Submit action triggered");
  lv_textarea_set_text(ui_InputWord, "");
  lv_label_set_text(ui_TxtWord, currentWord_.c_str());
  lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
  g_status->updateWiFiStatus(WiFiState::Working);
  currentResult_ = dictionaryApi_.lookupWord(currentWord_);
  g_status->updateWiFiStatus(g_network->isConnected() ? WiFiState::Ready : WiFiState::None);
  if (currentResult_.success) {
    g_audio->stop();
    currentWord_ = currentResult_.word;
    lv_obj_remove_flag(ui_Line, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_TxtWord, currentResult_.word.c_str());
    lv_label_set_text(ui_TxtExplanation, currentResult_.explanation.c_str());
    lv_label_set_text(ui_TxtSampleSentence, currentResult_.sampleSentence.c_str());
  } else {
    lv_label_set_text(ui_TxtExplanation, "Request failed. Please try again.");
    lv_obj_add_flag(ui_Line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_group_focus_obj(ui_InputWord);
    lv_textarea_set_text(ui_InputWord, currentWord_.c_str());
  }
}

void MainScreen::onKeyIn(char key) {
  if (!isScreenActive_) {
    return;
  }
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
  ESP_LOGD(TAG, "Function key input: %d", event.type);
  switch (event.type) {
  case FunctionKeyEvent::ReadWord:
    onPlayAudio("word");
    break;
  case FunctionKeyEvent::ReadExplanation:
    onPlayAudio("explanation");
    break;
  case FunctionKeyEvent::ReadSampleSentence:
    onPlayAudio("sample");
    break;
  case FunctionKeyEvent::WifiSettings:
    onWifiSettings();
    break;
  case FunctionKeyEvent::DownArrow:
    onDownArrow();
    break;
  case FunctionKeyEvent::UpArrow:
    onUpArrow();
    break;
  case FunctionKeyEvent::Escape:
    onEscape();
    break;
  default:
    break;
  }
}

void MainScreen::onPlayAudio(const String &audioType) {
  if (!isScreenActive_) {
    return;
  }
  if (currentWord_.length() == 0) {
    return;
  }
  AudioUrl audioUrl = dictionaryApi_.getAudioUrl(currentWord_, audioType);
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

void MainScreen::onWifiSettings() {
  ESP_LOGI(TAG, "Wifi settings");
  if (!isWifiSettings_) {
    wifiSettingsScreen_.initialize();
    wifiSettingsScreen_.setParent(this);
    wifiSettingsScreen_.scan();
    isWifiSettings_ = true;
    isScreenActive_ = false;
  } else {
    onBackFromWifiSettings();
  }
}

void MainScreen::onBackFromWifiSettings() {
  ESP_LOGI(TAG, "Back from wifi settings");
  if (isWifiSettings_) {
    wifiSettingsScreen_.shutdown();
    lv_disp_load_scr(ui_Main); // Switch back to main screen
    lv_group_focus_obj(ui_InputWord);
    isWifiSettings_ = false;
    isScreenActive_ = true;
  }
}

void MainScreen::onDownArrow() {
  if (ui_Result == nullptr)
    return;

  // Check if there's content below to scroll to
  int32_t scroll_bottom = lv_obj_get_scroll_bottom(ui_Result);
  if (scroll_bottom > 0) {
    // Scroll down by a fixed amount (e.g., 50 pixels)
    int32_t current_y = lv_obj_get_scroll_y(ui_Result);
    int32_t new_y = current_y + 50;

    // Don't scroll beyond the bottom
    int32_t max_scroll = lv_obj_get_scroll_top(ui_Result) + lv_obj_get_scroll_bottom(ui_Result);
    if (new_y > max_scroll)
      new_y = max_scroll;

    lv_obj_scroll_to_y(ui_Result, new_y, LV_ANIM_ON);
    ESP_LOGI(TAG, "Scrolled down to: %d", new_y);
  } else {
    ESP_LOGI(TAG, "Already at bottom");
  }
}

void MainScreen::onUpArrow() {
  if (ui_Result == nullptr)
    return;

  // Check if there's content above to scroll to
  int32_t scroll_top = lv_obj_get_scroll_top(ui_Result);
  if (scroll_top > 0) {
    // Scroll up by a fixed amount (e.g., 50 pixels)
    int32_t current_y = lv_obj_get_scroll_y(ui_Result);
    int32_t new_y = current_y - 50;

    // Don't scroll beyond the top
    if (new_y < 0)
      new_y = 0;

    lv_obj_scroll_to_y(ui_Result, new_y, LV_ANIM_ON);
    ESP_LOGI(TAG, "Scrolled up to: %d", new_y);
  } else {
    ESP_LOGI(TAG, "Already at top");
  }
}

void MainScreen::onEscape() {
  if (isWifiSettings_) {
    onBackFromWifiSettings();
    return;
  }
  if (isScreenActive_) {
    if (ui_InputWord) {
      if (!lv_obj_has_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN)) {
        lv_textarea_set_text(ui_InputWord, "");
      } else {
        lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(ui_InputWord);
      }
    }
  }
}

} // namespace dict