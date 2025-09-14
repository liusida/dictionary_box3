#include <Arduino.h>
#include "lvgl.h"
#include "TFT_eSPI.h"
#include "GT911.h"

#include "ui/ui.h"
#include "utils.h"
#include "drivers.h"

// Global objects
TFT_eSPI tft = TFT_eSPI();
GT911 touch = GT911();

void setup() {
  Serial.begin(115200);
  
  // Reset display and turn on backlight
  manualResetDisplay();

  // Initialize TFT display
  tft.init();
  tft.setRotation(3);

  // Initialize touch controller
  initTouch(touch);
  
  // Initialize LVGL
  lv_init();
  initLVGLDisplay(tft, touch);
  
  // Initialize UI and load splash screen
  ui_init();
  
  // Initialize BLE keyboard functionality
  initBLEKeyboard();
}

void loop() {
  // Handle LVGL tasks
  handleLVGLTasks();
  
  // Process queued keyboard events (safe for LVGL operations)
  processQueuedKeys();
  
  static unsigned long lastMemoryPrint = 0;
  unsigned long currentTime = millis();
  
  // Print memory status every 10 seconds
  // if (currentTime - lastMemoryPrint >= 10000) {
  //   printMemoryStatus();
  //   lastMemoryPrint = currentTime;
  // }
  
  delay(5); // Small delay for LVGL
}