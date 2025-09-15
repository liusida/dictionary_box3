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
 */
void sendKeyToLVGL(char key);

/**
 * @brief Process queued keys (call this from main loop)
 */
void processQueuedKeys();

void setSubmitCallback(void (*callback)());
void setKeyInCallback(void (*callback)(char key));