#include "drivers.h"
#include "ui/ui.h"

// BLE Keyboard related variables
static NimBLEScan* pBLEScan;
static std::vector<NimBLEAdvertisedDevice*> keyboardDevices;
static bool isScanning = false;
static bool isConnected = false;
static NimBLEClient* pClient = nullptr;
static NimBLERemoteService* pKeyboardService = nullptr;
static NimBLERemoteCharacteristic* pKeyboardChar = nullptr;

// LVGL callback functions for LVGL 9.x
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  TFT_eSPI* tft = (TFT_eSPI*)lv_display_get_user_data(disp);
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft->startWrite();
  tft->setAddrWindow(area->x1, area->y1, w, h);
  tft->pushPixels((uint16_t *)px_map, w * h);
  tft->endWrite();
  
  lv_display_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
  static uint32_t callback_count = 0;
  static uint32_t error_count = 0;
  static uint32_t last_error_time = 0;
  callback_count++;
  
  // Debug: Show that callback is being called
  if (callback_count % 1000 == 0) {
    Serial.printf("Touch callback called %d times, errors: %d\n", callback_count, error_count);
  }
  
  GT911* touch = (GT911*)lv_indev_get_user_data(indev_driver);
  
  // Add error handling for I2C communication
  bool touch_success = false;
  try {
    // Use GT911_MODE_INTERRUPT like the working example
    if (touch->touched(GT911_MODE_INTERRUPT)) {
      // Get touch points array like the working example
      GTPoint *tp = touch->getPoints();
      
      if (tp != nullptr) {
        // Use first touch point
        uint16_t x = tp[0].x;
        uint16_t y = tp[0].y;
        
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        touch_success = true;
        
        // Debug output (throttled to avoid spam)
        static uint32_t last_debug_time = 0;
        uint32_t now = millis();
        if (now - last_debug_time > 100) {
          Serial.printf("Touch detected: Point: (%d, %d)\n", x, y);
          last_debug_time = now;
        }
      }
    } else {
      data->state = LV_INDEV_STATE_RELEASED;
      touch_success = true;
    }
  } catch (...) {
    // Catch any exceptions from I2C communication
    error_count++;
    uint32_t now = millis();
    if (now - last_error_time > 5000) { // Only log every 5 seconds
      Serial.printf("Touch I2C error #%d (suppressing further errors for 5s)\n", error_count);
      last_error_time = now;
    }
  }
  
  // If touch reading failed, set released state
  if (!touch_success) {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Tick callback function for LVGL (like the working example)
static uint32_t my_tick(void) { 
  return millis(); 
}

bool initTouch(GT911& touch) {
  Serial.println("Initializing GT911 touch controller...");
  
  // Initialize I2C for touch with error handling
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  // Start with lower I2C frequency for better stability
  uint32_t i2c_freq = 50000; // Start with 50kHz instead of 100kHz
  Wire.setClock(i2c_freq);
  
  // Add pull-up resistors for better I2C stability
  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
  
  // Try touch initialization with retry
  bool touch_init = false;
  for (int retry = 0; retry < 3; retry++) {
    Serial.printf("Touch init attempt %d/3 at %dHz...\n", retry + 1, i2c_freq);
    touch_init = touch.begin(TOUCH_INT_PIN, TOUCH_RESET_PIN, TOUCH_I2C_ADDR, i2c_freq);
    if (touch_init) {
      break;
    }
    delay(100); // Wait before retry
  }
  
  if (touch_init) {
    Serial.println("Touch init successful");
    
    // Read touch info for debugging
    GTInfo* info = touch.readInfo();
    if (info) {
      Serial.printf("Touch resolution: %dx%d\n", info->xResolution, info->yResolution);
      Serial.printf("Product ID: %.4s\n", info->productId);
    }
    return true;
  } else {
    Serial.println("Touch init FAILED after 3 attempts!");
    return false;
  }
}

void initLVGLDisplay(TFT_eSPI& tft, GT911& touch) {
  Serial.println("Setting up LVGL display driver...");
  
  tft.setSwapBytes(true); // Replaces LV_COLOR_16_SWAP
  
  // Set tick callback like the working example
  Serial.println("Setting tick callback...");
  lv_tick_set_cb(my_tick);
  
  // Allocate double buffers in SPIRAM (PSRAM) like the working example
  #define BUF_ROWS 120
  static lv_color_t *buf1 = (lv_color_t*)ps_malloc(320 * BUF_ROWS * sizeof(lv_color_t));
  static lv_color_t *buf2 = (lv_color_t*)ps_malloc(320 * BUF_ROWS * sizeof(lv_color_t));
  
  if (!buf1 || !buf2) {
    Serial.println("ERROR: Failed to allocate buffers in SPIRAM!");
    return;
  }
  Serial.printf("Buffers allocated: buf1=%p, buf2=%p\n", (void*)buf1, (void*)buf2);
  
  // Create display with LVGL 9.x API
  lv_display_t *disp = lv_display_create(320, 240);
  if (disp == NULL) {
    Serial.println("ERROR: Display creation failed!");
    return;
  }
  
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_user_data(disp, &tft);
  
  // Use partial rendering mode like the working example
  lv_display_set_buffers(disp, buf1, buf2, 320 * BUF_ROWS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
  
  Serial.println("Setting up LVGL input driver...");
  
  // Set up LVGL input driver (touch) with LVGL 9.x API
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  lv_indev_set_user_data(indev, &touch);
  
  // Link input device to display (like the working example)
  lv_indev_set_display(indev, disp);
  
  Serial.println("LVGL display system initialized successfully!");
  Serial.println("Input driver linked to display");
}

void handleLVGLTasks() {
  lv_timer_handler();
}

// BLE Keyboard callback classes

class KeyboardClientCallbacks: public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pclient) {
        Serial.println("Connected to keyboard");
        isConnected = true;
        updateConnectButton("Connected");
    }
    
    void onDisconnect(NimBLEClient* pclient, int reason) {
        Serial.printf("Disconnected from keyboard, reason: %d\n", reason);
        isConnected = false;
        updateConnectButton("Connect");
    }
    
    bool onConnParamsUpdateRequest(NimBLEClient* pclient, const ble_gap_upd_params* params) {
        return true;
    }
    
    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
        Serial.println("Authentication complete");
    }
};

// Helper function to update dropdown with found devices
void updateKeyboardDropdown() {
    if (!ui_InputBLEs) return;
    
    String options = "";
    
    // Always clear and rebuild options
    if (keyboardDevices.size() == 0) {
        // No devices found - show placeholder
        options = "No keyboards found";
    } else {
        // Build device list
        for (size_t i = 0; i < keyboardDevices.size(); i++) {
            String deviceName = keyboardDevices[i]->getName().c_str();
            if (deviceName.length() == 0) {
                deviceName = "Unknown Device " + String(i + 1);
            }
            options += deviceName;
            if (i < keyboardDevices.size() - 1) {
                options += "\n";
            }
        }
    }
    
    // Always update the dropdown
    lv_dropdown_set_options(ui_InputBLEs, options.c_str());
    
    // Reset selection to first item
    lv_dropdown_set_selected(ui_InputBLEs, 0);
}

// Helper function to update connect button text
void updateConnectButton(const char* text) {
    if (ui_TxtConnectBLE) {
        lv_label_set_text(ui_TxtConnectBLE, text);
    }
}

// Function to send keyboard input to LVGL
void sendKeyToLVGL(char key) {
    if (key == 0) return; // Ignore null characters
    
    // Send to currently focused object
    lv_obj_t* focused = lv_group_get_focused(lv_group_get_default());
    if (focused) {
        // Check if it's a text area
        if (lv_obj_has_class(focused, &lv_textarea_class)) {
            // Handle text area
            if (key == 0x08) { // Backspace
                lv_textarea_delete_char(focused);
            } else if (key == '\n') { // Enter
                lv_textarea_add_char(focused, '\n');
            } else if (key >= 32 && key <= 126) { // Printable characters
                lv_textarea_add_char(focused, key);
            }
        }
        // Add support for other widget types as needed
    }
}

// HID key code to character conversion
char hidKeyToChar(uint8_t key, uint8_t modifier) {
    // Basic HID key code to character mapping
    // This is a simplified version - you may need to expand this
    
    // Check for modifier keys
    bool shift = (modifier & 0x02) != 0;  // Left Shift
    bool caps = (modifier & 0x02) != 0;   // Caps Lock (simplified)
    
    // Convert key codes to characters
    switch (key) {
        case 0x04: return shift ? 'A' : 'a';  // A
        case 0x05: return shift ? 'B' : 'b';  // B
        case 0x06: return shift ? 'C' : 'c';  // C
        case 0x07: return shift ? 'D' : 'd';  // D
        case 0x08: return shift ? 'E' : 'e';  // E
        case 0x09: return shift ? 'F' : 'f';  // F
        case 0x0A: return shift ? 'G' : 'g';  // G
        case 0x0B: return shift ? 'H' : 'h';  // H
        case 0x0C: return shift ? 'I' : 'i';  // I
        case 0x0D: return shift ? 'J' : 'j';  // J
        case 0x0E: return shift ? 'K' : 'k';  // K
        case 0x0F: return shift ? 'L' : 'l';  // L
        case 0x10: return shift ? 'M' : 'm';  // M
        case 0x11: return shift ? 'N' : 'n';  // N
        case 0x12: return shift ? 'O' : 'o';  // O
        case 0x13: return shift ? 'P' : 'p';  // P
        case 0x14: return shift ? 'Q' : 'q';  // Q
        case 0x15: return shift ? 'R' : 'r';  // R
        case 0x16: return shift ? 'S' : 's';  // S
        case 0x17: return shift ? 'T' : 't';  // T
        case 0x18: return shift ? 'U' : 'u';  // U
        case 0x19: return shift ? 'V' : 'v';  // V
        case 0x1A: return shift ? 'W' : 'w';  // W
        case 0x1B: return shift ? 'X' : 'x';  // X
        case 0x1C: return shift ? 'Y' : 'y';  // Y
        case 0x1D: return shift ? 'Z' : 'z';  // Z
        case 0x1E: return shift ? '!' : '1';  // 1
        case 0x1F: return shift ? '@' : '2';  // 2
        case 0x20: return shift ? '#' : '3';  // 3
        case 0x21: return shift ? '$' : '4';  // 4
        case 0x22: return shift ? '%' : '5';  // 5
        case 0x23: return shift ? '^' : '6';  // 6
        case 0x24: return shift ? '&' : '7';  // 7
        case 0x25: return shift ? '*' : '8';  // 8
        case 0x26: return shift ? '(' : '9';  // 9
        case 0x27: return shift ? ')' : '0';  // 0
        case 0x28: return '\n';               // Enter
        case 0x29: return 0x1B;               // Escape
        case 0x2A: return 0x08;               // Backspace
        case 0x2B: return '\t';               // Tab
        case 0x2C: return ' ';                // Space
        case 0x2D: return shift ? '_' : '-';  // -
        case 0x2E: return shift ? '+' : '=';  // =
        case 0x2F: return shift ? '{' : '[';  // [
        case 0x30: return shift ? '}' : ']';  // ]
        case 0x31: return shift ? '|' : '\\'; // \
        case 0x33: return shift ? ':' : ';';  // ;
        case 0x34: return shift ? '"' : '\''; // '
        case 0x35: return shift ? '~' : '`';  // `
        case 0x36: return shift ? '<' : ',';  // ,
        case 0x37: return shift ? '>' : '.';  // .
        case 0x38: return shift ? '?' : '/';  // /
        default: return 0; // Unknown key
    }
}

// Connect button event handler
void onConnectButtonClick(lv_event_t * e) {
    if (isConnected) {
        // Disconnect
        if (pClient && pClient->isConnected()) {
            pClient->disconnect();
        }
        return;
    }
    
    // Get selected device index
    uint16_t selected = lv_dropdown_get_selected(ui_InputBLEs);
    if (selected < keyboardDevices.size()) {
        connectBLEKeyboard(selected);
    }
}

// Scan button event handler
void onScanButtonClick(lv_event_t * e) {
    Serial.println("Scan button clicked");
    startBLEScan();
}

void initBLEKeyboard() {
    Serial.println("Initializing BLE Keyboard...");

    // Load keyboard settings screen
    lv_screen_load(ui_Keyboard_Settings);
    
    // Clear dropdown initially
    lv_dropdown_set_options(ui_InputBLEs, "Scanning for keyboards...");
    lv_dropdown_set_selected(ui_InputBLEs, 0);
    
    // Initialize NimBLE
    NimBLEDevice::init("ESP32-S3 Dictionary");
    NimBLEDevice::setPower(ESP_PWR_LVL_P3); // Low power for close range
    
    // Set security parameters for secure bonding
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);
    NimBLEDevice::setSecurityPasskey(123456); // Default passkey
    
    // Create BLE scan
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    // Set up button event handlers
    lv_obj_add_event_cb(ui_BtnConnectBLE, onConnectButtonClick, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_BtnScan, onScanButtonClick, LV_EVENT_CLICKED, NULL);
    
    // Start scanning
    startBLEScan();
}

void startBLEScan() {
    if (isScanning) return;
    
    Serial.println("Starting BLE scan for keyboards...");
    keyboardDevices.clear();
    
    // Show scanning status
    lv_dropdown_set_options(ui_InputBLEs, "Scanning...");
    lv_dropdown_set_selected(ui_InputBLEs, 0);
    updateConnectButton("Scanning...");
    
    isScanning = true;
    
    // Blocking scan - this will block for 1 second and return results
    NimBLEScanResults results = pBLEScan->getResults(1000, false); // 1 second, clear previous results
    
    // Process results
    for (int i = 0; i < results.getCount(); i++) {
        const NimBLEAdvertisedDevice* device = results.getDevice(i);
        
        // Check if device advertises HID service (keyboard)
        if (device->haveServiceUUID() && 
            device->isAdvertisingService(NimBLEUUID("1812"))) { // HID Service UUID
            Serial.printf("Found keyboard: %s\n", device->toString().c_str());
            keyboardDevices.push_back(new NimBLEAdvertisedDevice(*device));
        }
    }
    
    isScanning = false;
    
    // Update dropdown with results
    updateKeyboardDropdown();
    updateConnectButton("Connect");
    
    Serial.printf("Scan complete. Found %d keyboards\n", keyboardDevices.size());
}

void connectBLEKeyboard(uint8_t deviceIndex) {
    if (deviceIndex >= keyboardDevices.size()) {
        Serial.println("Invalid device index");
        return;
    }
    
    if (isConnected) {
        Serial.println("Already connected to a device");
        return;
    }
    
    Serial.printf("Connecting to device %d...\n", deviceIndex);
    updateConnectButton("Connecting...");
    
    // Create client
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(new KeyboardClientCallbacks());
    
    // Connect to device
    Serial.printf("Attempting to connect to device: %s\n", keyboardDevices[deviceIndex]->getAddress().toString().c_str());
    if (pClient->connect(keyboardDevices[deviceIndex])) {
        Serial.println("Connected to keyboard device");
        
        // Wait a moment for connection to stabilize
        delay(1000);
        
        // Discover all services and characteristics
        Serial.println("Discovering all services...");
        
        // Get all services (this should trigger service discovery)
        const std::vector<NimBLERemoteService*>& services = pClient->getServices();
        Serial.printf("Found %d services:\n", services.size());
        
        // If no services found, try to discover them explicitly
        if (services.size() == 0) {
            Serial.println("No services found, attempting explicit service discovery...");
            
            // Try to discover services explicitly
            if (pClient->discoverAttributes()) {
                Serial.println("Service discovery completed");
                const std::vector<NimBLERemoteService*>& discoveredServices = pClient->getServices();
                Serial.printf("Now found %d services:\n", discoveredServices.size());
            } else {
                Serial.println("Service discovery failed");
            }
        }
        
        // Use the services (either original or discovered)
        const std::vector<NimBLERemoteService*>& finalServices = pClient->getServices();
        Serial.printf("Processing %d services:\n", finalServices.size());
        
        for (const auto& service : finalServices) {
            Serial.printf("Service UUID: %s\n", service->getUUID().toString().c_str());
            
            // Get characteristics for this service
            const std::vector<NimBLERemoteCharacteristic*>& characteristics = service->getCharacteristics();
            Serial.printf("  Has %d characteristics:\n", characteristics.size());
            
            for (const auto& chr : characteristics) {
                Serial.printf("    - UUID: %s, canNotify: %s, canRead: %s, canWrite: %s\n", 
                             chr->getUUID().toString().c_str(),
                             chr->canNotify() ? "true" : "false",
                             chr->canRead() ? "true" : "false", 
                             chr->canWrite() ? "true" : "false");
                
                // Subscribe to any characteristic that supports notifications
                if (chr->canNotify()) {
                    Serial.printf("    -> Subscribing to notifications on %s\n", chr->getUUID().toString().c_str());
                    
                    bool subscribeResult = chr->subscribe(true, [](NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
                        // Only process HID service notifications
                        if (pChar->getRemoteService()->getUUID().equals(NimBLEUUID("1812"))) {
                            // Parse HID report (basic keyboard data)
                            if (length >= 8) {
                                uint8_t modifier = pData[0];
                                uint8_t key1 = pData[2];
                                uint8_t key2 = pData[3];
                                uint8_t key3 = pData[4];
                                uint8_t key4 = pData[5];
                                uint8_t key5 = pData[6];
                                uint8_t key6 = pData[7];
                                
                                // Only process if there's an actual key press (not release)
                                if (key1 != 0) {
                                    char key = hidKeyToChar(key1, modifier);
                                    if (key != 0) {
                                        Serial.printf("Key: %c", key);
                                        
                                        // Handle special keys
                                        if (key == '\n') {
                                            Serial.print(" (Enter)");
                                        } else if (key == 0x08) {
                                            Serial.print(" (Backspace)");
                                        } else if (key == '\t') {
                                            Serial.print(" (Tab)");
                                        } else if (key == 0x1B) {
                                            Serial.print(" (Escape)");
                                        }
                                        
                                        Serial.printf(" [0x%02X]\n", key1);
                                        
                                        // Send key to LVGL input system
                                        sendKeyToLVGL(key);
                                    }
                                }
                            }
                        }
                    });
                    
                    if (subscribeResult) {
                        Serial.printf("    -> Successfully subscribed to %s\n", chr->getUUID().toString().c_str());
                    } else {
                        Serial.printf("    -> Failed to subscribe to %s\n", chr->getUUID().toString().c_str());
                    }
                }
            }
        }
    } else {
        Serial.println("Failed to connect to keyboard");
        updateConnectButton("Connect");
    }
}
