#pragma once

#include "Preferences.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

#define BLE_SERVICE_UUID "1812"        // Keyboard Service UUID
#define BLE_CHARACTERISTIC_UUID "2a4d" // Actually, we subscribe to any characteristic that can notify...

/**
 * BLEKeyboard class - Handles BLE scanning, connection, and notification management
 *
 * This class provides a clean interface for BLE keyboard functionality:
 * - Scans for BLE devices in low power mode
 * - Connects to devices advertising the BLE_SERVICE_UUID
 * - Subscribes to notifications from all available characteristics
 * - Saves the last connected device address to persistent storage
 * - Automatically reconnects if the device is found again
 * - Keeps scanning if the device disconnects or cannot be found
 */
class BLEKeyboard {
  private:
    const NimBLEAdvertisedDevice *advDevice;
    bool doConnect;
    int powerLevel;
    uint32_t scanTimeMs;
    uint32_t scanRestartIntervalMs;
    Preferences preferences;
    NimBLEScan *pScan;

    // Forward declarations for nested callback classes
    class ClientCallbacks;
    class ScanCallbacks;

    // Callback instances (using pointers to avoid incomplete type issues)
    ClientCallbacks *clientCallbacks;
    ScanCallbacks *scanCallbacks;

    // Key callback function pointer
    typedef void (*KeyCallback)(char key);
    KeyCallback keyCallback;

    // Private methods
    static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    bool connectToServer();
    char convertKeyCodeToChar(uint8_t keyCode, uint8_t modifiers);

  public:
    /**
     * Constructor - Initializes the BLE keyboard with default settings
     */
    BLEKeyboard();

    /**
     * Destructor - Cleans up resources
     */
    ~BLEKeyboard();

    /**
     * Initialize the BLE keyboard - call this in setup()
     *
     * This method:
     * - Initializes Serial communication
     * - Sets up Preferences for persistent storage
     * - Initializes NimBLE with security settings
     * - Configures scanning parameters
     * - Starts scanning for BLE devices
     *
     * @param scanRestartIntervalMs Time to wait before restarting scan after it ends (default: 0ms)
     */
    void begin(uint32_t scanRestartIntervalMs = 0);

    /**
     * Main loop function - call this in loop()
     *
     * This method handles:
     * - Connection management
     * - Automatic reconnection
     * - Notification processing
     */
    void tick();

    /**
     * Get the current power level setting
     * @return Current power level in dBm
     */
    int getPowerLevel() const { return powerLevel; }

    /**
     * Set the transmit power level
     * @param level Power level in dBm (typically -15 to +9)
     */
    void setPowerLevel(int level) { powerLevel = level; }

    /**
     * Get the current scan time setting
     * @return Scan time in milliseconds
     */
    uint32_t getScanTime() const { return scanTimeMs; }

    /**
     * Set the scan time duration
     * @param timeMs Scan time in milliseconds (0 = scan forever)
     */
    void setScanTime(uint32_t timeMs) { scanTimeMs = timeMs; }

    /**
     * Set callback function for key events
     * @param callback Function to call when a key is received
     */
    void setKeyCallback(KeyCallback callback) { keyCallback = callback; }
    
    /**
     * Check if BLE keyboard is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;
};
