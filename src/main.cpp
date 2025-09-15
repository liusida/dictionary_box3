#include <Arduino.h>
#include "lvgl.h"
#include "TFT_eSPI.h"
#include "GT911.h"

#include "ui/ui.h"
#include "utils.h"
#include "drivers/drivers.h"

// Global objects
TFT_eSPI tft = TFT_eSPI();
GT911 touch = GT911();
BLEKeyboard bleKeyboard;

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
  lv_disp_load_scr(ui_WIFI_Settings);
  addObjectToDefaultGroup(ui_InputSSIDs);
  addObjectToDefaultGroup(ui_InputPassword);
  
  // Initialize BLE keyboard functionality
  bleKeyboard.setPowerLevel(-20);
  bleKeyboard.begin();
  
  // Set callback to connect BLE keyboard to LVGL key processing
  bleKeyboard.setKeyCallback(sendKeyToLVGL);
}

void loop() {
  // Handle LVGL tasks
  handleLVGLTasks();

  bleKeyboard.tick();
  
  // Process queued keyboard events (safe for LVGL operations)
  processQueuedKeys();
  
  delay(5); // Small delay for LVGL
}