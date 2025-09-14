#ifndef DRIVERS_H
#define DRIVERS_H

#include <Arduino.h>
#include "lvgl.h"
#include "TFT_eSPI.h"
#include "GT911.h"
#include "Wire.h"
#include "NimBLEDevice.h"

/**
 * @brief Initialize the GT911 touch controller
 * 
 * This function sets up:
 * - I2C communication for touch
 * - GT911 touch controller initialization
 * - Touch resolution and configuration
 * 
 * @param touch Reference to GT911 object
 * @return true if initialization successful, false otherwise
 */
bool initTouch(GT911& touch);

/**
 * @brief Initialize the LVGL display system
 * 
 * This function sets up:
 * - LVGL display driver with TFT_eSPI
 * - LVGL input driver with GT911 touch
 * - Display buffer and configuration
 * 
 * @param tft Reference to TFT_eSPI object
 * @param touch Reference to GT911 object
 */
void initLVGLDisplay(TFT_eSPI& tft, GT911& touch);

/**
 * @brief Handle LVGL tasks (call this in loop)
 */
void handleLVGLTasks();

/**
 * @brief Initialize BLE keyboard functionality
 * 
 * This function sets up:
 * - BLE device scanning for keyboards
 * - Secure bonding and connection
 * - UI integration for device selection
 */
void initBLEKeyboard();

/**
 * @brief Start BLE device scanning
 */
void startBLEScan();

/**
 * @brief Connect to selected BLE keyboard device
 * @param deviceIndex Index of selected device from dropdown
 */
void connectBLEKeyboard(uint8_t deviceIndex);

// Internal helper functions
void updateKeyboardDropdown();
void updateConnectButton(const char* text);
void onConnectButtonClick(lv_event_t * e);
void onScanButtonClick(lv_event_t * e);
char hidKeyToChar(uint8_t key, uint8_t modifier);
void sendKeyToLVGL(char key);

#endif // DRIVERS_H
