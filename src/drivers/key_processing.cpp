#include "key_processing.h"

// Key event queue for safe LVGL operations
KeyEvent pendingKeyEvent = {0, false};
void (*submitCallback)() = nullptr;
void (*keyInCallback)(char key) = nullptr;

// Function to queue keyboard input for safe processing in main loop
void sendKeyToLVGL(char key) {
    if (key == 0)
        return; // Ignore null characters

    // Store the key for processing in main loop (safe for BLE callback)
    pendingKeyEvent.key = key;
    pendingKeyEvent.valid = true;

    Serial.printf("Queued key: '%c' (0x%02X)\n", key, (unsigned char)key);
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

    Serial.printf("Processing key: '%c' (0x%02X), Group=%p, Focused=%p\n", key, (unsigned char)key, (void *)group, (void *)focused);

    if (focused) {
        // Check if it's a text area
        if (lv_obj_has_class(focused, &lv_textarea_class)) {
            Serial.println("Processing text area input");
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
            Serial.println("Focused object is not a text area");
        }
    } else {
        Serial.println("No focused object");
    }
}
