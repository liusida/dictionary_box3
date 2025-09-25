#include "ble_keyboard.h"
#include "log.h"

static const char *TAG = "BLE";

class BLEKeyboard::ClientCallbacks : public NimBLEClientCallbacks {
  public:
    BLEKeyboard *keyboard;
    ClientCallbacks(BLEKeyboard *kb) : keyboard(kb) {}
    void onConnect(NimBLEClient *pClient) override { ESP_LOGI(TAG, "Connected"); }
    void onDisconnect(NimBLEClient *pClient, int reason) override {
        ESP_LOGW(TAG, "%s Disconnected, reason = %d - Starting scan", pClient->getPeerAddress().toString().c_str(), reason);
        keyboard->advDevice = nullptr;
        keyboard->toKeyboardSettings = true;
        keyboard->pScan->start(keyboard->scanTimeMs, false, true);
    }
};

class BLEKeyboard::ScanCallbacks : public NimBLEScanCallbacks {
  public:
    BLEKeyboard *keyboard;
    ScanCallbacks(BLEKeyboard *kb) : keyboard(kb) {}
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
        bool found = false;
        String deviceName = advertisedDevice->getName().c_str();
        String deviceAddr = advertisedDevice->getAddress().toString().c_str();
        if (deviceName.length() > 0) {
            ESP_LOGD(TAG, "Found Device: %s (%s)", deviceName.c_str(), deviceAddr.c_str());
            keyboard->discoveredDevices.push_back(std::make_pair(deviceName, deviceAddr));
        } else {
            ESP_LOGD(TAG, "Found Device: %s", deviceAddr.c_str());
            keyboard->discoveredDevices.push_back(std::make_pair(deviceAddr, deviceAddr));
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
            keyboard->pScan->stop();
            keyboard->advDevice = advertisedDevice;
            keyboard->doConnect = true;
        }
    }
    void onScanEnd(const NimBLEScanResults &results, int reason) override {
        ESP_LOGI(TAG, "Scan Ended, reason: %d, device count: %d; Restarting scan", reason, results.getCount());
        keyboard->toKeyboardSettings = true;
    }
};

static BLEKeyboard *keyboardInstance = nullptr;

BLEKeyboard::BLEKeyboard()
    : advDevice(nullptr), doConnect(false), powerLevel(-15), scanTimeMs(500), scanRestartIntervalMs(0), clientCallbacks(nullptr),
      scanCallbacks(nullptr), keyCallback(nullptr), toKeyboardSettings(false) {
    clientCallbacks = new ClientCallbacks(this);
    scanCallbacks = new ScanCallbacks(this);
    keyboardInstance = this;
}

BLEKeyboard::~BLEKeyboard() {
    delete clientCallbacks;
    delete scanCallbacks;
    if (keyboardInstance == this) {
        keyboardInstance = nullptr;
    }
}

bool BLEKeyboard::initialize() {
    begin(0);
    return true;
}

void BLEKeyboard::shutdown() {
    if (pScan) {
        pScan->stop();
    }
    preferences.end();
}

void BLEKeyboard::tick() {
    if (doConnect) {
        doConnect = false;
        if (connectToServer()) {
            ESP_LOGI(TAG, "Success! we should now be getting notifications, no scanning for more!");
        } else {
            ESP_LOGW(TAG, "Failed to connect, no starting scan");
        }
    }
}

bool BLEKeyboard::isReady() const { return isConnected(); }

void BLEKeyboard::begin(uint32_t scanRestartIntervalMs) {
    this->scanRestartIntervalMs = scanRestartIntervalMs;
    preferences.end();
    if (!preferences.begin("ble_config", false)) {
        ESP_LOGE(TAG, "Failed to open preferences");
    } else {
        ESP_LOGI(TAG, "Preferences initialized successfully");
    }
    ESP_LOGI(TAG, "Starting NimBLE Client");
    NimBLEDevice::init("NimBLE-Client");
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    NimBLEDevice::setSecurityAuth(true, false, true);
    NimBLEDevice::setPower(powerLevel);
    pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(scanCallbacks, false);
    pScan->setInterval(100);
    pScan->setWindow(100);
    pScan->setActiveScan(true);
    pScan->start(scanTimeMs);
    ESP_LOGI(TAG, "Scanning for keyboard. Please press any key on the keyboard to wake it up.");
}

void BLEKeyboard::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    if (length >= 3) {
        ESP_LOGD("keypress", "BLE keyboard report: %d, %d, %d, %d, %d, %d, %d, %d", pData[0], pData[1], pData[2], pData[3], pData[4], pData[5],
                 pData[6], pData[7]);
        uint8_t modifiers = pData[0];
        uint8_t key1 = pData[2];
        if (key1 != 0x00) {
            if (keyboardInstance && keyboardInstance->keyCallback) {
                char key1_char = keyboardInstance->convertKeyCodeToChar(key1, modifiers);
                keyboardInstance->keyCallback(key1_char, key1, modifiers);
            }
        }
    }
}

bool BLEKeyboard::connectToServer() {
    NimBLEClient *pClient = nullptr;
    if (NimBLEDevice::getCreatedClientCount()) {
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(advDevice, false)) {
                ESP_LOGW(TAG, "Reconnect failed");
                return false;
            }
            ESP_LOGI(TAG, "Reconnected client");
        } else {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }
    if (!pClient) {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
            ESP_LOGE(TAG, "Max clients reached - no more connections available");
            return false;
        }
        pClient = NimBLEDevice::createClient();
        ESP_LOGI(TAG, "New client created");
        pClient->setClientCallbacks(clientCallbacks, false);
        pClient->setConnectionParams(12, 12, 0, 150);
        pClient->setConnectTimeout(5 * 1000);
        if (!pClient->connect(advDevice)) {
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

char BLEKeyboard::convertKeyCodeToChar(uint8_t keyCode, uint8_t modifiers) {
    bool shift = (modifiers & 0x02) != 0;
    bool caps = (modifiers & 0x02) != 0;
    switch (keyCode) {
    case 0x04:
        return shift ? 'A' : 'a';
    case 0x05:
        return shift ? 'B' : 'b';
    case 0x06:
        return shift ? 'C' : 'c';
    case 0x07:
        return shift ? 'D' : 'd';
    case 0x08:
        return shift ? 'E' : 'e';
    case 0x09:
        return shift ? 'F' : 'f';
    case 0x0A:
        return shift ? 'G' : 'g';
    case 0x0B:
        return shift ? 'H' : 'h';
    case 0x0C:
        return shift ? 'I' : 'i';
    case 0x0D:
        return shift ? 'J' : 'j';
    case 0x0E:
        return shift ? 'K' : 'k';
    case 0x0F:
        return shift ? 'L' : 'l';
    case 0x10:
        return shift ? 'M' : 'm';
    case 0x11:
        return shift ? 'N' : 'n';
    case 0x12:
        return shift ? 'O' : 'o';
    case 0x13:
        return shift ? 'P' : 'p';
    case 0x14:
        return shift ? 'Q' : 'q';
    case 0x15:
        return shift ? 'R' : 'r';
    case 0x16:
        return shift ? 'S' : 's';
    case 0x17:
        return shift ? 'T' : 't';
    case 0x18:
        return shift ? 'U' : 'u';
    case 0x19:
        return shift ? 'V' : 'v';
    case 0x1A:
        return shift ? 'W' : 'w';
    case 0x1B:
        return shift ? 'X' : 'x';
    case 0x1C:
        return shift ? 'Y' : 'y';
    case 0x1D:
        return shift ? 'Z' : 'z';
    case 0x1E:
        return shift ? '!' : '1';
    case 0x1F:
        return shift ? '@' : '2';
    case 0x20:
        return shift ? '#' : '3';
    case 0x21:
        return shift ? '$' : '4';
    case 0x22:
        return shift ? '%' : '5';
    case 0x23:
        return shift ? '^' : '6';
    case 0x24:
        return shift ? '&' : '7';
    case 0x25:
        return shift ? '*' : '8';
    case 0x26:
        return shift ? '(' : '9';
    case 0x27:
        return shift ? ')' : '0';
    case 0x28:
        return '\n';
    case 0x29:
        return 0x08;
    case 0x2A:
        return 0x08;
    case 0x2B:
        return '\t';
    case 0x2C:
        return ' ';
    case 0x2D:
        return shift ? '_' : '-';
    case 0x2E:
        return shift ? '+' : '=';
    case 0x2F:
        return shift ? '{' : '[';
    case 0x30:
        return shift ? '}' : ']';
    case 0x31:
        return shift ? '|' : '\\';
    case 0x32:
        return shift ? '~' : '`';
    case 0x33:
        return shift ? ':' : ';';
    case 0x34:
        return shift ? '"' : '\'';
    case 0x35:
        return shift ? '~' : '`';
    case 0x36:
        return shift ? '<' : ',';
    case 0x37:
        return shift ? '>' : '.';
    case 0x38:
        return shift ? '?' : '/';
    default:
        return 0;
    }
}

bool BLEKeyboard::isConnected() const {
    ESP_LOGD(TAG, "advDevice: %p", advDevice);
    delay(100);
    if (advDevice) {
        NimBLEClient *pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient && pClient->isConnected()) {
            return true;
        }
    }
    return false;
}

void BLEKeyboard::startScan() {
    ESP_LOGI(TAG, "Starting BLE scan...");
    discoveredDevices.clear();
    if (pScan) {
        pScan->start(scanTimeMs, false, true);
    } else {
        ESP_LOGE(TAG, "Scan object not initialized");
    }
}

std::vector<String> BLEKeyboard::getDiscoveredDevices() {
    std::vector<String> deviceNames;
    for (const auto& device : discoveredDevices) {
        deviceNames.push_back(device.first);
    }
    return deviceNames;
}

bool BLEKeyboard::connectToDevice(const String& deviceName) {
    ESP_LOGI(TAG, "Attempting to connect to device: %s", deviceName.c_str());
    for (const auto& device : discoveredDevices) {
        if (device.first == deviceName) {
            ESP_LOGI(TAG, "Found device %s with address %s", device.first.c_str(), device.second.c_str());
            preferences.putString("addr", device.second);
            doConnect = true;
            advDevice = nullptr;
            return true;
        }
    }
    ESP_LOGW(TAG, "Device %s not found in discovered devices", deviceName.c_str());
    return false;
}


