#pragma once
#include "common.h"
#include "key_processor.h"
#include "psram_allocator.h"
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <functional>

#define BLE_SERVICE_UUID "1812"        // Keyboard Service UUID
#define BLE_CHARACTERISTIC_UUID "2a4d" // Actually, we subscribe to any characteristic that can notify...

namespace dict {

using KeyCallback = std::function<void(char key, uint8_t keyCode, uint8_t modifiers)>;

class BLEKeyboard {
public:
  // Singleton access
  static BLEKeyboard &instance(); // Get singleton instance

  // Core lifecycle methods
  bool initialize();    // Initialize BLE keyboard connection and scanning
  void shutdown();      // Clean shutdown of BLE keyboard
  void tick();          // Process BLE keyboard events and connection state
  bool isReady() const; // Check if BLE keyboard is ready for use

  // Main functionality methods
  void begin(); // Start BLE scanning with optional restart interval
  void startScan();                               // Begin scanning for BLE keyboard devices
  bool isScanning() const { return scanning_; }
  uint32_t getScanStartTime() const { return scanStartTime_; }
  uint32_t getScanEndTime() const { return scanEndTime_; }
  bool isConnected() const;                                                    // Check if connected to a BLE keyboard
  void setKeyCallback(const KeyCallback &callback) { keyCallback = callback; } // Set callback for key events

private:
  // Private constructor/destructor for singleton
  BLEKeyboard();
  ~BLEKeyboard() = default;
  BLEKeyboard(const BLEKeyboard &) = delete;
  BLEKeyboard &operator=(const BLEKeyboard &) = delete;
  
  // Private member variables
  class ClientCallbacks;
  class ScanCallbacks;
  
  bool initialized_;
  bool scanning_;
  uint32_t scanStartTime_;
  uint32_t scanEndTime_;
  String advDeviceAddress;
  bool doConnect;
  int powerLevel;
  uint32_t scanTimeMs;
  Preferences preferences;
  NimBLEScan *pScan;

  ClientCallbacks *clientCallbacks;
  ScanCallbacks *scanCallbacks;

  KeyCallback keyCallback;
  bool capsLockOn_ = false; // track CapsLock state

  KeyProcessor *keyProcessor_;

  // Private methods
  static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
  bool connectToServer();
  char convertKeyCodeToChar(uint8_t keyCode, uint8_t modifiers);
};

} // namespace dict
