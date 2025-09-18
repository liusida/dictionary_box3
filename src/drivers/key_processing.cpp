#include "key_processing.h"
#include "main.h"
#include "main_screen_control.h"
#include "audio_manager.h"
#include "esp_log.h"
#include "utils.h"

extern AudioManager audio;
extern AppState currentState;

static const char *TAG = "KeyProc";

// ------------------- KeyProcessing class -------------------

KeyProcessing::KeyProcessing()
  : pendingKeyEvent{0, false}, funcHead(0), funcTail(0), submitCallback(nullptr), keyInCallback(nullptr) {
  for (int i = 0; i < kFunctionQueueSize; ++i) {
    funcQueue[i] = KeyFunctionAction::None;
  }
}

void KeyProcessing::begin() {
}

void KeyProcessing::setSubmitCallback(void (*callback)()) { submitCallback = callback; }
void KeyProcessing::setKeyInCallback(void (*callback)(char key)) { keyInCallback = callback; }

void KeyProcessing::enqueueFunction(KeyFunctionAction action) {
  uint8_t nextHead = (funcHead + 1) % kFunctionQueueSize;
  if (nextHead == funcTail) {
    return;
  }
  funcQueue[funcHead] = action;
  funcHead = nextHead;
}

bool KeyProcessing::dequeueFunction(KeyFunctionAction &actionOut) {
  if (funcHead == funcTail) {
    return false;
  }
  actionOut = funcQueue[funcTail];
  funcQueue[funcTail] = KeyFunctionAction::None;
  funcTail = (funcTail + 1) % kFunctionQueueSize;
  return true;
}

void KeyProcessing::sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers) {
  if (key != 0) {
    pendingKeyEvent.key = key;
    pendingKeyEvent.valid = true;
    ESP_LOGD(TAG, "Queued key: '%c' (0x%02X)", key, (unsigned char)key);
  } else {
    ESP_LOGD("keypress", "Functional key: %d, %d", key1, modifiers);
    if (key1 == 67) {
      enqueueFunction(KeyFunctionAction::VolumeDown);
    } else if (key1 == 68) {
      enqueueFunction(KeyFunctionAction::VolumeUp);
    } else if (key1 == 58) { // F1
      enqueueFunction(KeyFunctionAction::PrintMemoryStatus);
    } else if (key1 == 59) {
      enqueueFunction(KeyFunctionAction::ReadWord);
    } else if (key1 == 60) {
      enqueueFunction(KeyFunctionAction::ReadExplanation);
    } else if (key1 == 61) {
      enqueueFunction(KeyFunctionAction::ReadSampleSentence);
    }
  }
}

void KeyProcessing::processQueuedKeys() {
  if (!pendingKeyEvent.valid)
    return;

  char key = pendingKeyEvent.key;
  pendingKeyEvent.valid = false;

  lv_group_t *group = lv_group_get_default();
  lv_obj_t *focused = lv_group_get_focused(group);

  ESP_LOGD(TAG, "Processing key: '%c' (0x%02X), Group=%p, Focused=%p", key, (unsigned char)key, (void *)group, (void *)focused);

  if (focused) {
    if (lv_obj_has_class(focused, &lv_textarea_class)) {
      ESP_LOGD(TAG, "Processing text area input");
      if (key == 0x08) {
        lv_textarea_delete_char(focused);
        if (keyInCallback) {
          keyInCallback(key);
        }
      } else if (key == '\n') {
        lv_textarea_t *ta = (lv_textarea_t *)focused;
        if (ta->one_line) {
          if (submitCallback) {
            submitCallback();
          }
        } else {
          lv_textarea_add_char(focused, '\n');
        }
      } else if (key >= 32 && key <= 126) {
        lv_textarea_add_char(focused, key);
        if (keyInCallback) {
          keyInCallback(key);
        }
      }
    } else {
      ESP_LOGV(TAG, "Focused object is not a text area");
    }
  } else {
    ESP_LOGV(TAG, "No focused object");
  }
}

void KeyProcessing::processQueuedFunctions() {
  static float currentVolume = 0.7f;
  KeyFunctionAction action;
  while (dequeueFunction(action)) {
    switch (action) {
      case KeyFunctionAction::VolumeDown:
        currentVolume -= 0.05f;
        if (currentVolume < 0.0f) currentVolume = 0.0f;
        ESP_LOGD(TAG, "Volume: %f", currentVolume);
        audio.setVolume(currentVolume);
        break;
      case KeyFunctionAction::VolumeUp:
        currentVolume += 0.05f;
        if (currentVolume > 1.0f) currentVolume = 1.0f;
        ESP_LOGD(TAG, "Volume: %f", currentVolume);
        audio.setVolume(currentVolume);
        break;
      case KeyFunctionAction::PrintMemoryStatus:
        printMemoryStatus();
        break;
      case KeyFunctionAction::ReadWord:
        if (currentState == STATE_MAIN) {
          readWord();
        }
        break;
      case KeyFunctionAction::ReadExplanation:
        if (currentState == STATE_MAIN) {
          readExplanation();
        }
        break;
      case KeyFunctionAction::ReadSampleSentence:
        if (currentState == STATE_MAIN) {
          readSampleSentence();
        }
        break;
      default:
        break;
    }
  }
}

void KeyProcessing::tick() {
  processQueuedKeys();
  processQueuedFunctions();
}

// ------------------- Global instance & wrappers -------------------

static KeyProcessing g_keyProcessing;

KeyProcessing& getKeyProcessing() { return g_keyProcessing; }

// Legacy wrappers
void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers) { g_keyProcessing.sendKeyToLVGL(key, key1, modifiers); }
void processQueuedKeys() { g_keyProcessing.processQueuedKeys(); }
void setSubmitCallback(void (*callback)()) { g_keyProcessing.setSubmitCallback(callback); }
void setKeyInCallback(void (*callback)(char key)) { g_keyProcessing.setKeyInCallback(callback); }
