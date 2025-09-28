#pragma once
#include "common.h"
#include "psram_allocator.h"
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <functional>

#define BLE_SERVICE_UUID "1812"        // Keyboard Service UUID
#define BLE_CHARACTERISTIC_UUID "2a4d" // Actually, we subscribe to any characteristic that can notify...

namespace dict {

using KeyCallback = std::function<void(char key, uint8_t keyCode, uint8_t modifiers)>;

class BLEKeyboard {
  public:
    // Constructor/Destructor
    BLEKeyboard();
    ~BLEKeyboard();

    // Core lifecycle methods
    bool initialize(); // Initialize BLE keyboard connection and scanning
    void shutdown(); // Clean shutdown of BLE keyboard
    void tick(); // Process BLE keyboard events and connection state
    bool isReady() const; // Check if BLE keyboard is ready for use

    // Main functionality methods
    void begin(uint32_t scanRestartIntervalMs = 0); // Start BLE scanning with optional restart interval
    void startScan(); // Begin scanning for BLE keyboard devices
    bool isScanning() const { return scanning_; }
    uint32_t getScanStartTime() const { return scanStartTime_; }
    uint32_t getScanEndTime() const { return scanEndTime_; }
    bool isConnected() const; // Check if connected to a BLE keyboard
    bool connectToDevice(const String& deviceName); // Connect to specific device by name
    void setKeyCallback(const KeyCallback& callback) { keyCallback = callback; } // Set callback for key events

    // Utility/getter methods
    int getPowerLevel() const { return powerLevel; } // Get current BLE power level
    void setPowerLevel(int level) { powerLevel = level; } // Set BLE power level
    uint32_t getScanTime() const { return scanTimeMs; } // Get scan duration in milliseconds
    void setScanTime(uint32_t timeMs) { scanTimeMs = timeMs; } // Set scan duration
    std::vector<String> getDiscoveredDevices(); // Get list of discovered device names

  protected:
    std::vector<std::pair<String, String>, PsramAllocator<std::pair<String, String>>> discoveredDevices;

  private:
    // Private member variables
    bool initialized_;
    bool scanning_;
    uint32_t scanStartTime_;
    uint32_t scanEndTime_;
    String advDeviceAddress;
    bool doConnect;
    int powerLevel;
    uint32_t scanTimeMs;
    uint32_t scanRestartIntervalMs;
    bool toKeyboardSettings;
    Preferences preferences;
    NimBLEScan *pScan;

    class ClientCallbacks;
    class ScanCallbacks;

    ClientCallbacks *clientCallbacks;
    ScanCallbacks *scanCallbacks;

    KeyCallback keyCallback;

    // Private methods
    static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    bool connectToServer();
    char convertKeyCodeToChar(uint8_t keyCode, uint8_t modifiers);
};

} // namespace dict


