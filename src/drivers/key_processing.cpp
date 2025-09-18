#include "key_processing.h"
#include "main.h"
#include "main_screen_control.h"
#include "audio_manager.h"
#include "esp_log.h"

extern AudioManager audio;
extern AppState currentState;

// Key event queue for safe LVGL operations
KeyEvent pendingKeyEvent = {0, false};
void (*submitCallback)() = nullptr;
void (*keyInCallback)(char key) = nullptr;

static const char *TAG = "KeyProc";

// Function to queue keyboard input for safe processing in main loop
void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers) {
    if (key != 0) {
        // Store the key for processing in main loop (safe for BLE callback)
        pendingKeyEvent.key = key;
        pendingKeyEvent.valid = true;

        ESP_LOGD(TAG, "Queued key: '%c' (0x%02X)", key, (unsigned char)key);
    } else {
        ESP_LOGD("keypress", "Functional key: %d, %d", key1, modifiers);
        static float currentVolume = 0.7f; // 0.0 .. 1.0
        if (key1 == 67) { // F10
            currentVolume -= 0.05f;
            if (currentVolume < 0.0f) currentVolume = 0.0f;
            ESP_LOGD(TAG, "Volume: %f", currentVolume);
            audio.setVolume(currentVolume);
        } else if (key1 == 68) { // F11
            currentVolume += 0.05f;
            if (currentVolume > 1.0f) currentVolume = 1.0f;
            ESP_LOGD(TAG, "Volume: %f", currentVolume);
            audio.setVolume(currentVolume);
        } else if (key1 == 59) { // F2
            if (currentState == STATE_MAIN) {
                readWord();
            }
        } else if (key1 == 60) { // F3
            if (currentState == STATE_MAIN) {
                readExplanation();
            }
        } else if (key1 == 61) { // F4
            if (currentState == STATE_MAIN) {
                readSampleSentence();
            }
        }
    }
}

void setSubmitCallback(void (*callback)()) { submitCallback = callback; }
void setKeyInCallback(void (*callback)(char key)) { keyInCallback = callback; }

// Function to process queued keys (call this from main loop)
void processQueuedKeys() {
    if (!pendingKeyEvent.valid)
        return;

    char key = pendingKeyEvent.key;
    pendingKeyEvent.valid = false; // Clear the event

    // Now do the LVGL operations safely in main loop context
    lv_group_t *group = lv_group_get_default();
    lv_obj_t *focused = lv_group_get_focused(group);

    ESP_LOGD(TAG, "Processing key: '%c' (0x%02X), Group=%p, Focused=%p", key, (unsigned char)key, (void *)group, (void *)focused);

    if (focused) {
        // Check if it's a text area
        if (lv_obj_has_class(focused, &lv_textarea_class)) {
            ESP_LOGD(TAG, "Processing text area input");
            // Handle text area
            if (key == 0x08) { // Backspace
                lv_textarea_delete_char(focused);
                if (keyInCallback) {
                    keyInCallback(key);
                }
            } else if (key == '\n') { // Enter
                lv_textarea_t *ta = (lv_textarea_t *)focused;
                if (ta->one_line) {
                    // call a callback (submit the form)
                    if (submitCallback) {
                        submitCallback();
                    }
                } else {
                    lv_textarea_add_char(focused, '\n');
                }
            } else if (key >= 32 && key <= 126) { // Printable characters
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
