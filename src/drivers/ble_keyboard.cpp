#include "ble_keyboard.h"
#include "esp_log.h"
static const char *TAG = "BLE";

// Client callbacks implementation
class BLEKeyboard::ClientCallbacks : public NimBLEClientCallbacks {
public:
    BLEKeyboard* keyboard;
    
    ClientCallbacks(BLEKeyboard* kb) : keyboard(kb) {}
    
    void onConnect(NimBLEClient *pClient) override { 
        ESP_LOGI(TAG, "Connected"); 
    }

    void onDisconnect(NimBLEClient *pClient, int reason) override {
        ESP_LOGW(TAG, "%s Disconnected, reason = %d - Starting scan", 
                     pClient->getPeerAddress().toString().c_str(), reason);
        keyboard->pScan->start(keyboard->scanTimeMs, false, true);
    }
};

// Scan callbacks implementation
class BLEKeyboard::ScanCallbacks : public NimBLEScanCallbacks {
public:
    BLEKeyboard* keyboard;
    
    ScanCallbacks(BLEKeyboard* kb) : keyboard(kb) {}
    
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
        bool found = false;
        // Print only name (if exists) and address
        String deviceName = advertisedDevice->getName().c_str();
        String deviceAddr = advertisedDevice->getAddress().toString().c_str();
        
        if (deviceName.length() > 0) {
            ESP_LOGD(TAG, "Found Device: %s (%s)", deviceName.c_str(), deviceAddr.c_str());
        } else {
            ESP_LOGD(TAG, "Found Device: %s", deviceAddr.c_str());
        }
        
        if (keyboard->preferences.getString("addr").equals(String(advertisedDevice->getAddress().toString().c_str()))) {
            found = true;
        }
        if (advertisedDevice->isAdvertisingService(NimBLEUUID(BLE_SERVICE_UUID))) {
            found = true;
        }
        if (found) {
            ESP_LOGI(TAG, "Found Our Service");
            keyboard->preferences.putString("addr", advertisedDevice->getAddress().toString().c_str());
            /** stop scan before connecting */
            keyboard->pScan->stop();
            /** Save the device reference in a global for the client to use*/
            keyboard->advDevice = advertisedDevice;
            /** Ready to connect now */
            keyboard->doConnect = true;
        }
    }

    /** Callback to process the results of the completed scan or restart it */
    void onScanEnd(const NimBLEScanResults &results, int reason) override {
        ESP_LOGI(TAG, "Scan Ended, reason: %d, device count: %d; Restarting scan", reason, results.getCount());
        delay(keyboard->scanRestartIntervalMs); // wait before starting scan again
        keyboard->pScan->start(keyboard->scanTimeMs, false, true);
    }
};

// BLEKeyboard implementation

// Static instance pointer for notifyCB access
static BLEKeyboard* keyboardInstance = nullptr;

BLEKeyboard::BLEKeyboard() : advDevice(nullptr), doConnect(false), powerLevel(-15), scanTimeMs(500), 
                            scanRestartIntervalMs(0), clientCallbacks(nullptr), scanCallbacks(nullptr), keyCallback(nullptr) {
    // Initialize callback objects
    clientCallbacks = new ClientCallbacks(this);
    scanCallbacks = new ScanCallbacks(this);
    
    // Set static instance pointer
    keyboardInstance = this;
}

BLEKeyboard::~BLEKeyboard() {
    // Clean up callback objects
    delete clientCallbacks;
    delete scanCallbacks;
    
    // Clear static instance pointer
    if (keyboardInstance == this) {
        keyboardInstance = nullptr;
    }
}

void BLEKeyboard::begin(uint32_t scanRestartIntervalMs) {
    // Store the scan restart interval
    this->scanRestartIntervalMs = scanRestartIntervalMs;
    // Initialize Preferences to access NVS (Non-Volatile Storage)
    if (!preferences.begin("ble_config", false)) {
        ESP_LOGE(TAG, "Failed to open preferences");
    } else {
        ESP_LOGI(TAG, "Preferences initialized successfully");
    }

    ESP_LOGI(TAG, "Starting NimBLE Client");

    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    NimBLEDevice::setSecurityAuth(true, false, true);

    /** Optional: set the transmit power */
    NimBLEDevice::setPower(powerLevel);
    pScan = NimBLEDevice::getScan();

    /** Set the callbacks to call when scan events occur, no duplicates */
    pScan->setScanCallbacks(scanCallbacks, false);

    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(100);
    pScan->setWindow(100);

    /**
     * Active scan will gather scan response data from advertisers
     *  but will use more energy from both devices
     */
    pScan->setActiveScan(true);

    /** Start scanning for advertisers */
    pScan->start(scanTimeMs);
    ESP_LOGI(TAG, "Scanning for peripherals");
}

void BLEKeyboard::tick() {
    /** Loop here until we find a device we want to connect to */
    if (doConnect) {
        doConnect = false;
        /** Found a device we want to connect to, do it now */
        if (connectToServer()) {
            ESP_LOGI(TAG, "Success! we should now be getting notifications, no scanning for more!");
        } else {
            ESP_LOGW(TAG, "Failed to connect, no starting scan");
        }
    }
}

void BLEKeyboard::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    // std::string str = (isNotify == true) ? "Notification" : "Indication";
    // str += " from ";
    // str += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
    // str += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    // str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
    // str += ", Handle = " + std::to_string(pRemoteCharacteristic->getHandle());
    // str += ", Value = " + std::string((char *)pData, length);
    // Serial.printf("%s\n", str.c_str());
    
    // Parse BLE keyboard data and call callback if set
    if (length > 0 && pData[0] == 0x00) { // Standard keyboard report
        if (length >= 3) {
            // BLE keyboard reports: [modifiers, reserved, key1, key2, key3, key4, key5, key6]
            ESP_LOGD("keypress", "BLE keyboard report: %d, %d, %d, %d, %d, %d, %d, %d", pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7]);
            uint8_t modifiers = pData[0];
            uint8_t key1 = pData[2];
            
            if (key1 != 0x00) { // Key pressed
                if (keyboardInstance && keyboardInstance->keyCallback) {
                    char key1_char = keyboardInstance->convertKeyCodeToChar(key1, modifiers);
                    keyboardInstance->keyCallback(key1_char, key1, modifiers);
                }
            }
        }
    }
}

bool BLEKeyboard::connectToServer() {
    NimBLEClient *pClient = nullptr;

    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount()) {
        /**
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(advDevice, false)) {
                ESP_LOGW(TAG, "Reconnect failed");
                return false;
            }
            ESP_LOGI(TAG, "Reconnected client");
        } else {
            /**
             *  We don't already have a client that knows this device,
             *  check for a client that is disconnected that we can use.
             */
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient) {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
            ESP_LOGE(TAG, "Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        ESP_LOGI(TAG, "New client created");

        pClient->setClientCallbacks(clientCallbacks, false);
        /**
         *  Set initial connection parameters:
         *  These settings are safe for 3 clients to connect reliably, can go faster
         * if you have less connections. Timeout should be a multiple of the
         * interval, minimum is 100ms. Min interval: 12 * 1.25ms = 15, Max interval:
         * 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
         */
        pClient->setConnectionParams(12, 12, 0, 150);

        /** Set how long we are willing to wait for the connection to complete
         * (milliseconds), default is 30000. */
        pClient->setConnectTimeout(5 * 1000);

        if (!pClient->connect(advDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it
             * has no data */
            NimBLEDevice::deleteClient(pClient);
            ESP_LOGW(TAG, "Failed to connect, deleted client");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            ESP_LOGW(TAG, "Failed to connect");
            return false;
        }
    }

    ESP_LOGI(TAG, "Connected to: %s RSSI: %d", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    NimBLERemoteService *pSvc = nullptr;

    pSvc = pClient->getService(BLE_SERVICE_UUID);
    if (pSvc) {
        std::vector<NimBLERemoteCharacteristic *> pChars = pSvc->getCharacteristics(true);
        for (const auto &chr : pChars) {
            if (chr->canNotify()) {
                ESP_LOGI(TAG, "Subscribing to Characteristic UUID: %s, Handle: %d", chr->getUUID().toString().c_str(), chr->getHandle());
                chr->subscribe(true, notifyCB);
            }
        }
    } else {
        ESP_LOGW(TAG, "%s service not found.", BLE_SERVICE_UUID);
    }

    ESP_LOGI(TAG, "Done with this device!");
    return true;
}

// Helper function to convert key codes to characters
char BLEKeyboard::convertKeyCodeToChar(uint8_t keyCode, uint8_t modifiers) {
    bool shift = (modifiers & 0x02) != 0; // Left shift
    bool caps = (modifiers & 0x02) != 0;  // Caps lock
    
    switch (keyCode) {
        case 0x04: return shift ? 'A' : 'a'; // A
        case 0x05: return shift ? 'B' : 'b'; // B
        case 0x06: return shift ? 'C' : 'c'; // C
        case 0x07: return shift ? 'D' : 'd'; // D
        case 0x08: return shift ? 'E' : 'e'; // E
        case 0x09: return shift ? 'F' : 'f'; // F
        case 0x0A: return shift ? 'G' : 'g'; // G
        case 0x0B: return shift ? 'H' : 'h'; // H
        case 0x0C: return shift ? 'I' : 'i'; // I
        case 0x0D: return shift ? 'J' : 'j'; // J
        case 0x0E: return shift ? 'K' : 'k'; // K
        case 0x0F: return shift ? 'L' : 'l'; // L
        case 0x10: return shift ? 'M' : 'm'; // M
        case 0x11: return shift ? 'N' : 'n'; // N
        case 0x12: return shift ? 'O' : 'o'; // O
        case 0x13: return shift ? 'P' : 'p'; // P
        case 0x14: return shift ? 'Q' : 'q'; // Q
        case 0x15: return shift ? 'R' : 'r'; // R
        case 0x16: return shift ? 'S' : 's'; // S
        case 0x17: return shift ? 'T' : 't'; // T
        case 0x18: return shift ? 'U' : 'u'; // U
        case 0x19: return shift ? 'V' : 'v'; // V
        case 0x1A: return shift ? 'W' : 'w'; // W
        case 0x1B: return shift ? 'X' : 'x'; // X
        case 0x1C: return shift ? 'Y' : 'y'; // Y
        case 0x1D: return shift ? 'Z' : 'z'; // Z
        case 0x1E: return shift ? '!' : '1'; // 1
        case 0x1F: return shift ? '@' : '2'; // 2
        case 0x20: return shift ? '#' : '3'; // 3
        case 0x21: return shift ? '$' : '4'; // 4
        case 0x22: return shift ? '%' : '5'; // 5
        case 0x23: return shift ? '^' : '6'; // 6
        case 0x24: return shift ? '&' : '7'; // 7
        case 0x25: return shift ? '*' : '8'; // 8
        case 0x26: return shift ? '(' : '9'; // 9
        case 0x27: return shift ? ')' : '0'; // 0
        case 0x28: return '\n'; // Enter
        case 0x29: return 0x08; // Escape
        case 0x2A: return 0x08; // Backspace
        case 0x2B: return '\t'; // Tab
        case 0x2C: return ' ';  // Space
        case 0x2D: return shift ? '_' : '-'; // -
        case 0x2E: return shift ? '+' : '='; // =
        case 0x2F: return shift ? '{' : '['; // [
        case 0x30: return shift ? '}' : ']'; // ]
        case 0x31: return shift ? '|' : '\\'; // \
        case 0x32: return shift ? '~' : '`'; // `
        case 0x33: return shift ? ':' : ';'; // ;
        case 0x34: return shift ? '"' : '\''; // '
        case 0x35: return shift ? '~' : '`'; // `
        case 0x36: return shift ? '<' : ','; // ,
        case 0x37: return shift ? '>' : '.'; // .
        case 0x38: return shift ? '?' : '/'; // /
        default: return 0; // Unknown key
    }
}

bool BLEKeyboard::isConnected() const {
    // Check if we have a connected client for our advertised device
    if (advDevice) {
        NimBLEClient* pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient && pClient->isConnected()) {
            return true;
        }
    }
    
    return false;
}
