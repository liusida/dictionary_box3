#pragma once

#include <Arduino.h>
#include "lvgl.h"

// Key event queue for safe LVGL operations
struct KeyEvent {
  char key;
  bool valid;
};

enum class KeyFunctionAction : uint8_t {
  None = 0,
  VolumeDown,
  VolumeUp,
  PrintMemoryStatus,
  ReadWord,
  ReadExplanation,
  ReadSampleSentence
};

class KeyProcessing {
public:
  KeyProcessing();

  // lifecycle
  void begin();
  void tick(); // drains both key and function queues

  // enqueue events from ISR/callback contexts
  void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers);

  // optional callbacks for text input handling
  void setSubmitCallback(void (*callback)());
  void setKeyInCallback(void (*callback)(char key));

  // expose for rare direct use
  void processQueuedKeys();
  void processQueuedFunctions();

private:
  // key event (single-slot) written by callbacks, drained in main loop
  volatile KeyEvent pendingKeyEvent;

  // ring buffer for function actions to avoid deep call stacks
  static const int kFunctionQueueSize = 8;
  volatile uint8_t funcHead;
  volatile uint8_t funcTail;
  volatile KeyFunctionAction funcQueue[kFunctionQueueSize];

  // callbacks
  void (*submitCallback)();
  void (*keyInCallback)(char key);

  // helpers
  void enqueueFunction(KeyFunctionAction action);
  bool dequeueFunction(KeyFunctionAction &actionOut);
};

// Global instance accessors (lightweight wrappers for existing code)
KeyProcessing& getKeyProcessing();

// Backward-compatible C-style wrappers
void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers);
void processQueuedKeys();
void setSubmitCallback(void (*callback)());
void setKeyInCallback(void (*callback)(char key));