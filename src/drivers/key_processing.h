#pragma once

#include <Arduino.h>
#include "lvgl.h"

// Key event queue for safe LVGL operations
struct KeyEvent {
  char key;
  bool valid;
};

extern KeyEvent pendingKeyEvent;

/**
 * @brief Send key to LVGL for processing (thread-safe)
 * @param key Character to send
 * @param key1 HID key code
 * @param modifiers Modifier mask
 */
void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers);

/**
 * @brief Process queued keys (call this from main loop)
 */
void processQueuedKeys();

void setSubmitCallback(void (*callback)());
void setKeyInCallback(void (*callback)(char key));