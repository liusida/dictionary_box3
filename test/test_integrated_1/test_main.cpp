#include <Arduino.h>
#include <unity.h>
#include "lvgl_memory.h"
#include "display_manager.h"
#include "ui_status.h"
#include "ble_keyboard.h"
#include "key_processor.h"
#include "network_control.h"
#include "audio_manager.h"
#include "dictionary_api.h"
#include "lvgl_helper.h"
#include "test_wifi_credentials.h"
#include "screens/main_screen.h"
#include "ui/ui.h"
#include "utils.h"

using namespace dict;

namespace dict {
// Global objects for the integrated test
DisplayManager* g_display = nullptr;
StatusOverlay* g_status = nullptr;
BLEKeyboard* g_bleKeyboard = nullptr;
KeyProcessor* g_keyProcessor = nullptr;
NetworkControl* g_network = nullptr;
AudioManager* g_audio = nullptr;
} // namespace dict

MainScreen* g_mainScreen = nullptr;

// Test state tracking
bool g_bleConnected = false;
bool g_wifiConnected = false;
bool g_audioReady = false;


void setup() {
    Serial.begin(115200);
    delay(2000); // Give time for serial to initialize
    // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Debug);
    ESP_LOGI("MemoryTest", "Start...");
    printMemoryStatus();
    //delay(1000);
    
    ESP_LOGI("INTEGRATED_TEST", "Starting integrated test setup...");
    
    // Initialize display manager
    g_display = new DisplayManager();
    TEST_ASSERT_TRUE_MESSAGE(g_display->initialize(), "Display manager initialize failed");
    TEST_ASSERT_TRUE(g_display->isReady());
    ESP_LOGI("INTEGRATED_TEST", "Display manager initialized successfully");

    ESP_LOGI("MemoryTest", "After display manager...");
    printMemoryStatus();
    //delay(1000);

    lvglEnableKeyEventHandler();

    ESP_LOGI("MemoryTest", "After lvglEnableKeyEventHandler...");
    printMemoryStatus();
    //delay(1000);

    // ui_init();
    ui_Main_screen_init();
    ESP_LOGI("MemoryTest", "After ui_init...");
    printMemoryStatus();
    //delay(1000);

    g_mainScreen = new MainScreen();
    TEST_ASSERT_TRUE_MESSAGE(g_mainScreen->initialize(), "Main screen initialize failed");
    TEST_ASSERT_TRUE(g_mainScreen->isReady());
    g_mainScreen->show();
    ESP_LOGI("INTEGRATED_TEST", "Main UI screen loaded");
    ESP_LOGI("MemoryTest", "After main screen...");
    printMemoryStatus();
    //delay(1000);
    
    // Initialize status overlay and attach to main screen
    g_status = new StatusOverlay();
    TEST_ASSERT_TRUE_MESSAGE(g_status->initialize(), "Status overlay initialize failed");
    TEST_ASSERT_TRUE(g_status->isReady());
    
    g_status->attachToScreen(ui_Main);
    g_status->show();
    g_status->setPosition(LV_ALIGN_TOP_RIGHT, -10, 10);
    ESP_LOGI("INTEGRATED_TEST", "Status overlay initialized and attached");
    ESP_LOGI("MemoryTest", "After status overlay...");
    printMemoryStatus();
    //delay(1000);
    
    // Initially hide all status icons until services are ready
    g_status->updateWiFiStatus(false);
    g_status->updateBLEStatus(false);
    g_status->updateAudioStatus(false);
    ESP_LOGI("INTEGRATED_TEST", "Status icons hidden initially");
    
    // Initialize BLE keyboard
    g_bleKeyboard = new BLEKeyboard();
    g_keyProcessor = new KeyProcessor();
    TEST_ASSERT_TRUE_MESSAGE(g_bleKeyboard->initialize(), "BLE keyboard initialize failed");
    TEST_ASSERT_TRUE_MESSAGE(g_keyProcessor->initialize(), "Key processor initialize failed");
    
    // Set up BLE keyboard callback to send keys to event system
    g_bleKeyboard->setKeyCallback([&](char ch, uint8_t keyCode, uint8_t modifiers){
        g_keyProcessor->sendKeyToLVGL(ch, keyCode, modifiers);
    });
    
    g_bleKeyboard->begin();
    ESP_LOGI("INTEGRATED_TEST", "BLE keyboard initialized, scanning for devices...");
    ESP_LOGI("MemoryTest", "After ble keyboard...");
    printMemoryStatus();
    //delay(1000);
    
    // Initialize network control
    g_network = new NetworkControl();
    TEST_ASSERT_TRUE_MESSAGE(g_network->initialize(), "Network control initialize failed");
    g_network->begin();
    g_network->connectToNetwork(TEST_WIFI_SSID, TEST_WIFI_PASSWORD);
    ESP_LOGI("INTEGRATED_TEST", "Network control initialized, attempting WiFi connection...");
    ESP_LOGI("MemoryTest", "After network control...");
    printMemoryStatus();
    //delay(1000);


    // Initialize audio manager
    g_audio = new AudioManager();
    TEST_ASSERT_TRUE_MESSAGE(g_audio->initialize(), "Audio manager initialize failed");
    ESP_LOGI("INTEGRATED_TEST", "Audio manager initialized");
    ESP_LOGI("MemoryTest", "After audio manager...");
    printMemoryStatus();
    // delay(1000);

    ESP_LOGI("INTEGRATED_TEST", "Setup completed successfully!");
}

void loop() {
    // Process display updates
    if (g_display && g_display->isReady()) {
        g_display->tick();
    }

    // Process status overlay updates
    if (g_status && g_status->isReady()) {
        g_status->tick();
    }
    
    // Process BLE keyboard
    if (g_bleKeyboard && g_bleKeyboard->isReady()) {
        g_bleKeyboard->tick();
        
        // Check BLE connection status and update UI
        bool bleConnected = g_bleKeyboard->isConnected();
        if (bleConnected != g_bleConnected) {
            g_bleConnected = bleConnected;
            if (g_status) {
                if (bleConnected) {
                    g_status->updateBLEStatus(true, "BLE Keyboard");
                    ESP_LOGI("INTEGRATED_TEST", "BLE keyboard connected!");
                    ESP_LOGI("MemoryTest", "After ble keyboard connected...");
                    printMemoryStatus();
                    //delay(1000);
                } else {
                    g_status->updateBLEStatus(false);
                    ESP_LOGI("INTEGRATED_TEST", "BLE keyboard disconnected");
                }
            }
        }
        if (!bleConnected && !g_bleKeyboard->isScanning() && millis() - g_bleKeyboard->getScanEndTime() > 1000) {
            ESP_LOGI("INTEGRATED_TEST", "When not connected and scan ended more than 1 second ago, start scan again");
            g_bleKeyboard->startScan();
        }
    }

    // Process key processor
    if (g_keyProcessor && g_keyProcessor->isReady()) {
        g_keyProcessor->tick();
    }
    
    // Process all events in the event system
    EventSystem::instance().processAllEvents();
    
    // Process network control
    if (g_network && g_network->isReady()) {
        g_network->tick();
        
        // Check WiFi connection status and update UI
        bool wifiConnected = g_network->isConnected();
        if (wifiConnected != g_wifiConnected) {
            g_wifiConnected = wifiConnected;
            if (g_status) {
                if (wifiConnected) {
                    String ssid = WiFi.SSID();
                    g_status->updateWiFiStatus(true, ssid);
                    ESP_LOGI("INTEGRATED_TEST", "WiFi connected to: %s", ssid.c_str());
                    g_mainScreen->onConnectionReady();
                    ESP_LOGI("MemoryTest", "After wifi connected...");
                    printMemoryStatus();
                    //delay(1000);
                
                } else {
                    g_status->updateWiFiStatus(false);
                    ESP_LOGI("INTEGRATED_TEST", "WiFi disconnected");
                }
            }
        }
        if (!wifiConnected && !g_network->isConnecting() && millis() - g_network->getConnectEndTime() > 10000) {
            ESP_LOGI("INTEGRATED_TEST", "When not connected and connect ended more than 10 seconds ago, start connect again");
            g_network->randomizeMACAddress();
            g_network->connectToNetwork(TEST_WIFI_SSID, TEST_WIFI_PASSWORD);
        } else {
            static uint32_t lastCheck = 0;
            while (millis() - lastCheck > 1000) {
                lastCheck = millis();
                // ESP_LOGD("INTEGRATED_TEST", "Check why not hit reconnect: %d, %d, %d", wifiConnected, g_network->isConnecting(), millis() - g_network->getConnectEndTime());
            }
        }
    }
    
    // Process audio manager
    if (g_audio && g_audio->isReady()) {
        g_audio->tick();
        
        // Check audio readiness and update UI
        bool audioReady = g_audio->isReady();
        if (audioReady != g_audioReady) {
            g_audioReady = audioReady;
            if (g_status) {
                if (audioReady) {
                    g_status->updateAudioStatus(true);
                    ESP_LOGI("INTEGRATED_TEST", "Audio system ready");
                    ESP_LOGI("MemoryTest", "After audio system ready...");
                    printMemoryStatus();
                    //delay(1000);
                } else {
                    g_status->updateAudioStatus(false);
                    ESP_LOGI("INTEGRATED_TEST", "Audio system not ready");
                }
            }
        }
    }
    
    // Process main screen
    if (g_mainScreen && g_mainScreen->isReady()) {
        g_mainScreen->tick();
    }
    
    delay(10); // Small delay to prevent overwhelming the system
}

void tearDown() {
    ESP_LOGI("INTEGRATED_TEST", "Starting teardown...");
    
    // Clean up in reverse order
    if (g_audio) {
        g_audio->shutdown();
        delete g_audio;
        g_audio = nullptr;
    }
    
    if (g_network) {
        g_network->shutdown();
        delete g_network;
        g_network = nullptr;
    }
    
    if (g_bleKeyboard) {
        g_bleKeyboard->shutdown();
        delete g_bleKeyboard;
        g_bleKeyboard = nullptr;
    }
    
    if (g_status) {
        g_status->shutdown();
        delete g_status;
        g_status = nullptr;
    }
    
    if (g_display) {
        g_display->shutdown();
        delete g_display;
        g_display = nullptr;
    }
    
    ESP_LOGI("INTEGRATED_TEST", "Teardown completed");
}
