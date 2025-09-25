#pragma once

#include "Preferences.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <functional>

#define BLE_SERVICE_UUID "1812"        // Keyboard Service UUID
#define BLE_CHARACTERISTIC_UUID "2a4d" // Actually, we subscribe to any characteristic that can notify...

class BLEKeyboard {
  private:
    const NimBLEAdvertisedDevice *advDevice;
    bool doConnect;
    int powerLevel;
    uint32_t scanTimeMs;
    uint32_t scanRestartIntervalMs;
    Preferences preferences;
    NimBLEScan *pScan;

    class ClientCallbacks;
    class ScanCallbacks;

    ClientCallbacks *clientCallbacks;
    ScanCallbacks *scanCallbacks;

    using KeyCallback = std::function<void(char key, uint8_t keyCode, uint8_t modifiers)>;
    KeyCallback keyCallback;

    static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    bool connectToServer();
    char convertKeyCodeToChar(uint8_t keyCode, uint8_t modifiers);

  public:
    BLEKeyboard();
    ~BLEKeyboard();

    bool initialize();
    void shutdown();
    void tick();
    bool isReady() const;
    void begin(uint32_t scanRestartIntervalMs = 0);
    int getPowerLevel() const { return powerLevel; }
    void setPowerLevel(int level) { powerLevel = level; }
    uint32_t getScanTime() const { return scanTimeMs; }
    void setScanTime(uint32_t timeMs) { scanTimeMs = timeMs; }
    void setKeyCallback(const KeyCallback& callback) { keyCallback = callback; }
    bool isConnected() const;
    void startScan();
    std::vector<String> getDiscoveredDevices();
    bool connectToDevice(const String& deviceName);

  protected:
    bool toKeyboardSettings;
    std::vector<std::pair<String, String>> discoveredDevices;
};


