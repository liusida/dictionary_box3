/**
 * NimBLE Client Demo
 * 
 * This example demonstrates a low-power BLE client that automatically connects
 * to a specific device and maintains the connection. It was modified from the
 * original NimBLE-Arduino example at:
 * https://github.com/h2zero/NimBLE-Arduino/blob/master/examples/NimBLE_Client/NimBLE_Client.ino
 * 
 * Features:
 * - Scans for BLE devices in low power mode
 * - Connects to devices advertising the BLE_SERVICE_UUID
 * - Subscribes to notifications from all available characteristics
 * - Saves the last connected device address to persistent storage
 * - Automatically reconnects if the device is found again
 * - Keeps scanning if the device disconnects or cannot be found
 */

 #include "Preferences.h"
 #include <Arduino.h>
 #include <NimBLEDevice.h>

 #define BLE_SERVICE_UUID "1812"        // Keyboard Service UUID
 #define BLE_CHARACTERISTIC_UUID "2a4d" // Actually, we subscribe to any characteristic that can notify...
 
 static const NimBLEAdvertisedDevice *advDevice;
 static bool doConnect = false;
 static int powerLevel = -15; /** -15dbm */
 static uint32_t scanTimeMs = 500; /** scan time in milliseconds, 0 = scan forever */
 static Preferences preferences;
 /**  None of these are required as they will be handled by the library with
  *defaults. **
  **                       Remove as you see fit for your needs */
 class ClientCallbacks : public NimBLEClientCallbacks {
     void onConnect(NimBLEClient *pClient) override { Serial.printf("Connected\n"); }
 
     void onDisconnect(NimBLEClient *pClient, int reason) override {
         Serial.printf("%s Disconnected, reason = %d - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
         NimBLEDevice::getScan()->start(scanTimeMs, false, true);
     }
 
     /********************* Security handled here *********************/
     void onPassKeyEntry(NimBLEConnInfo &connInfo) override {
         Serial.printf("Server Passkey Entry\n");
         /**
          * This should prompt the user to enter the passkey displayed
          * on the peer device.
          */
         NimBLEDevice::injectPassKey(connInfo, 123456);
     }
 
     void onConfirmPasskey(NimBLEConnInfo &connInfo, uint32_t pass_key) override {
         Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
         /** Inject false if passkeys don't match. */
         NimBLEDevice::injectConfirmPasskey(connInfo, true);
     }
 
     /** Pairing process complete, we can check the results in connInfo */
     void onAuthenticationComplete(NimBLEConnInfo &connInfo) override {
         if (!connInfo.isEncrypted()) {
             Serial.printf("Encrypt connection failed - disconnecting\n");
             /** Find the client with the connection handle provided in connInfo */
             NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
             return;
         }
     }
 } clientCallbacks;
 
 /** Define a class to handle the callbacks when scan events are received */
 class ScanCallbacks : public NimBLEScanCallbacks {
     void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
         bool found = false;
         Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
         //  Serial.printf(">> Last Device Address: %s\n",
         //  preferences.getString("addr").c_str()); Serial.printf(">> Advertised
         //  Device Address: %s\n",
         //  advertisedDevice->getAddress().toString().c_str());
         if (preferences.getString("addr").equals(String(advertisedDevice->getAddress().toString().c_str()))) {
             found = true;
         }
         if (advertisedDevice->isAdvertisingService(NimBLEUUID(BLE_SERVICE_UUID))) {
             found = true;
         }
         if (found) {
             Serial.printf("Found Our Service\n");
             preferences.putString("addr", advertisedDevice->getAddress().toString().c_str());
             //  Serial.printf("Saved Last Device Address: %s\n",
             //  preferences.getString("addr").c_str());
             /** stop scan before connecting */
             NimBLEDevice::getScan()->stop();
             /** Save the device reference in a global for the client to use*/
             advDevice = advertisedDevice;
             /** Ready to connect now */
             doConnect = true;
         }
     }
 
     /** Callback to process the results of the completed scan or restart it */
     void onScanEnd(const NimBLEScanResults &results, int reason) override {
         Serial.printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
         delay(10000); // wait 10 seconds before starting scan again
         NimBLEDevice::getScan()->start(scanTimeMs, false, true);
     }
 } scanCallbacks;
 
 /** Notification / Indication receiving handler callback */
 void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
     std::string str = (isNotify == true) ? "Notification" : "Indication";
     str += " from ";
     str += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
     str += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
     str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
     str += ", Handle = " + std::to_string(pRemoteCharacteristic->getHandle());
     str += ", Value = " + std::string((char *)pData, length);
     Serial.printf("%s\n", str.c_str());
 }
 
 /** Handles the provisioning of clients and connects / interfaces with the
  * server */
 bool connectToServer() {
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
                 Serial.printf("Reconnect failed\n");
                 return false;
             }
             Serial.printf("Reconnected client\n");
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
             Serial.printf("Max clients reached - no more connections available\n");
             return false;
         }
 
         pClient = NimBLEDevice::createClient();
 
         Serial.printf("New client created\n");
 
         pClient->setClientCallbacks(&clientCallbacks, false);
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
             Serial.printf("Failed to connect, deleted client\n");
             return false;
         }
     }
 
     if (!pClient->isConnected()) {
         if (!pClient->connect(advDevice)) {
             Serial.printf("Failed to connect\n");
             return false;
         }
     }
 
     Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
 
     NimBLERemoteService *pSvc = nullptr;
 
     pSvc = pClient->getService(BLE_SERVICE_UUID);
     if (pSvc) {
         std::vector<NimBLERemoteCharacteristic *> pChars = pSvc->getCharacteristics(true);
         for (const auto &chr : pChars) {
             if (chr->canNotify()) {
                 Serial.printf("Subscribing to Characteristic UUID: %s, Handle: %d\n", chr->getUUID().toString().c_str(), chr->getHandle());
                 chr->subscribe(true, notifyCB);
             }
         }
     } else {
         Serial.printf("1812 service not found.\n");
     }
 
     Serial.printf("Done with this device!\n");
     return true;
 }
 
 void setup() {
     Serial.begin(115200);
     delay(2000);
 
     // Initialize Preferences to access NVS (Non-Volatile Storage)
     if (!preferences.begin("ble_config", false)) {
         Serial.println("Failed to open preferences");
     } else {
         Serial.println("Preferences initialized successfully");
     }
 
     Serial.printf("Starting NimBLE Client\n");
 
     /** Initialize NimBLE and set the device name */
     NimBLEDevice::init("NimBLE-Client");
     NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
 
     NimBLEDevice::setSecurityAuth(true, false, true);
 
     /** Optional: set the transmit power */
     NimBLEDevice::setPower(powerLevel);
     NimBLEScan *pScan = NimBLEDevice::getScan();
 
     /** Set the callbacks to call when scan events occur, no duplicates */
     pScan->setScanCallbacks(&scanCallbacks, false);
 
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
     Serial.printf("Scanning for peripherals\n");
 }
 
 void loop() {
     /** Loop here until we find a device we want to connect to */
     delay(10);

     if (doConnect) {
         doConnect = false;
         /** Found a device we want to connect to, do it now */
         if (connectToServer()) {
             Serial.printf("Success! we should now be getting notifications, no "
                           "scanning for more!\n");
         } else {
             Serial.printf("Failed to connect, no starting scan\n");
         }
     }
 }