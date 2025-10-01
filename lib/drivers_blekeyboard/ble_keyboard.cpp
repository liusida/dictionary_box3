#include "ble_keyboard.h"
#include "core_misc/log.h"

namespace dict {

static const char *TAG = "BLE";

class BLEKeyboard::ClientCallbacks : public NimBLEClientCallbacks {
public:
  BLEKeyboard *keyboard;
  ClientCallbacks(BLEKeyboard *kb) : keyboard(kb) {}
  void onConnect(NimBLEClient *pClient) override { ESP_LOGI(TAG, "Connected"); }
  void onDisconnect(NimBLEClient *pClient, int reason) override {
    ESP_LOGW(TAG, "%s Disconnected, reason = %d - Starting scan", pClient->getPeerAddress().toString().c_str(), reason);
    keyboard->advDeviceAddress = "";
    keyboard->pScan->start(keyboard->scanTimeMs, false, true);
  }
};

class BLEKeyboard::ScanCallbacks : public NimBLEScanCallbacks {
public:
  BLEKeyboard *keyboard;
  ScanCallbacks(BLEKeyboard *kb) : keyboard(kb) {}
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
    String deviceName = advertisedDevice->getName().c_str();
    String deviceAddr = advertisedDevice->getAddress().toString().c_str();
    bool hasService = false;
    try {
      hasService = advertisedDevice->isAdvertisingService(NimBLEUUID(BLE_SERVICE_UUID));
    } catch (...) {
      ESP_LOGW(TAG, "Service check failed for: %s", deviceAddr.c_str());
      hasService = false;
    }

    bool found = false;
    if (deviceName.length() > 0) {
      ESP_LOGD(TAG, "Found Device: %s (%s)", deviceName.c_str(), deviceAddr.c_str());
    } else {
      ESP_LOGD(TAG, "Found Device: %s", deviceAddr.c_str());
    }
    if (keyboard->preferences.getString("addr").equals(deviceAddr)) {
      found = true;
    }
    if (hasService) {
      found = true;
    }
    if (found) {
      ESP_LOGI(TAG, "Found Our Service");
      keyboard->preferences.putString("addr", deviceAddr);
      keyboard->pScan->stop();
      keyboard->advDeviceAddress = deviceAddr;
      keyboard->doConnect = true;
    }
  }
  void onScanEnd(const NimBLEScanResults &results, int reason) override {
    ESP_LOGI(TAG, "Scan Ended, reason: %d, device count: %d; Restarting scan", reason, results.getCount());
    keyboard->scanning_ = false;
    keyboard->scanEndTime_ = millis();
  }
};

static BLEKeyboard *keyboardInstance = nullptr;

BLEKeyboard &BLEKeyboard::instance() {
  static BLEKeyboard instance;
  return instance;
}

BLEKeyboard::BLEKeyboard()
    : initialized_(false), scanning_(false), scanStartTime_(0), scanEndTime_(0), advDeviceAddress(""), doConnect(false), powerLevel(-15),
      scanTimeMs(500), keyCallback(nullptr) {
  clientCallbacks = new ClientCallbacks(this);
  scanCallbacks = new ScanCallbacks(this);
  keyProcessor_ = new KeyProcessor();
  keyboardInstance = this;
}

bool BLEKeyboard::initialize() {
  begin();
  keyProcessor_->initialize();
  setKeyCallback([&](char ch, uint8_t keyCode, uint8_t modifiers) { keyProcessor_->sendKeyToLVGL(ch, keyCode, modifiers); });
  initialized_ = true;
  return true;
}

void BLEKeyboard::shutdown() {
  initialized_ = false;
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
  if (keyProcessor_ && keyProcessor_->isReady()) {
    keyProcessor_->tick();
  }
}

bool BLEKeyboard::isReady() const { return initialized_; }

void BLEKeyboard::begin() {
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
    ESP_LOGD("keypress", "BLE keyboard report: %d, %d, %d, %d, %d, %d, %d, %d", pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6],
             pData[7]);
    uint8_t modifiers = pData[0];
    uint8_t key1 = pData[2];
    if (key1 != 0x00) {
      // Handle CapsLock key (HID 0x39) - toggle state and swallow event
      if (key1 == 0x39) { // HID: CapsLock
        keyboardInstance->capsLockOn_ = !keyboardInstance->capsLockOn_;
        return; // swallow CapsLock key
      }

      if (keyboardInstance && keyboardInstance->keyCallback) {
        char key1_char = keyboardInstance->convertKeyCodeToChar(key1, modifiers);
        keyboardInstance->keyCallback(key1_char, key1, modifiers);
      }
    }
  }
}

bool BLEKeyboard::connectToServer() {
  NimBLEClient *pClient = nullptr;
  if (advDeviceAddress.isEmpty()) {
    ESP_LOGW(TAG, "No device address to connect to");
    return false;
  }
  NimBLEAddress targetAddress(advDeviceAddress.c_str(), BLE_ADDR_PUBLIC);
  if (NimBLEDevice::getCreatedClientCount()) {
    pClient = NimBLEDevice::getClientByPeerAddress(targetAddress);
    if (pClient) {
      if (!pClient->connect(targetAddress, false)) {
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
    if (!pClient->connect(targetAddress)) {
      NimBLEDevice::deleteClient(pClient);
      ESP_LOGW(TAG, "Failed to connect, deleted client");
      return false;
    }
  }
  if (!pClient->isConnected()) {
    if (!pClient->connect(targetAddress)) {
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
  const uint8_t kShiftMask = 0x02 | 0x20; // LeftShift | RightShift
  bool shift = (modifiers & kShiftMask) != 0;

  // Letters A..Z (HID 0x04..0x1D)
  if (keyCode >= 0x04 && keyCode <= 0x1D) {
    char base = static_cast<char>('a' + (keyCode - 0x04));
    bool upper = shift ^ capsLockOn_;
    return upper ? static_cast<char>(base - ('a' - 'A')) : base;
  }

  // existing switch for all other keys remains unchanged
  switch (keyCode) {
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
    return 0; // ESCAPE, go through to function key handler
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
  if (advDeviceAddress.length() > 0) {
    NimBLEClient *pClient = NimBLEDevice::getClientByPeerAddress(NimBLEAddress(advDeviceAddress.c_str(), BLE_ADDR_PUBLIC));
    if (pClient && pClient->isConnected()) {
      return true;
    }
  }
  uint32_t t0 = millis();
  if (millis() - t0 > 5000) {
    ESP_LOGD(TAG, "advDevice: %s", advDeviceAddress.c_str());
  }
  return false;
}

void BLEKeyboard::startScan() {
  ESP_LOGI(TAG, "Starting BLE scan...");
  if (pScan) {
    scanning_ = true;
    scanStartTime_ = millis();
    scanEndTime_ = 0;
    pScan->start(scanTimeMs, false, true);
  } else {
    ESP_LOGE(TAG, "Scan object not initialized");
  }
}

} // namespace dict
