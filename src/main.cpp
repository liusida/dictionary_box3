#include "audio_manager.h"
#include "ble_keyboard.h"
#include "dictionary_api.h"
#include "display_manager.h"
#include "key_processor.h"
#include "lvgl_helper.h"
#include "lvgl_memory.h"
#include "network_control.h"
#include "screens/main_screen.h"
#include "test_wifi_credentials.h"
#include "ui.h"
#include "ui_status.h"
#include "utils.h"
#include <Arduino.h>
#include <unity.h>

using namespace dict;

// Test state tracking
bool g_bleConnected = false;
bool g_wifiConnected = false;
bool g_audioReady = false;

// #define BOOT_MEMORY_ANALYSIS(msg) ESP_LOGI("MemoryTest", msg); printMemoryStatus();
#define BOOT_MEMORY_ANALYSIS(msg) ;

void setup() {
  Serial.begin(115200);
  delay(2000); // Give time for serial to initialize
  // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Debug);
  BOOT_MEMORY_ANALYSIS("Start...");

  ESP_LOGI("INTEGRATED_TEST", "Starting integrated test setup...");

  // Initialize display manager
  TEST_ASSERT_TRUE_MESSAGE(DisplayManager::instance().initialize(), "Display manager initialize failed");
  TEST_ASSERT_TRUE(DisplayManager::instance().isReady());
  ESP_LOGI("INTEGRATED_TEST", "Display manager initialized successfully");

  BOOT_MEMORY_ANALYSIS("After display manager...");

  lvglEnableKeyEventHandler();

  BOOT_MEMORY_ANALYSIS("After lvglEnableKeyEventHandler...");

  // ui_init();
  ui_Main_screen_init();
  BOOT_MEMORY_ANALYSIS("After ui_init...");

  TEST_ASSERT_TRUE_MESSAGE(MainScreen::instance().initialize(), "Main screen initialize failed");
  TEST_ASSERT_TRUE(MainScreen::instance().isReady());
  MainScreen::instance().show();
  ESP_LOGI("INTEGRATED_TEST", "Main UI screen loaded");
  BOOT_MEMORY_ANALYSIS("After main screen...");

  // Initialize status overlay and attach to main screen
  TEST_ASSERT_TRUE_MESSAGE(StatusOverlay::instance().initialize(), "Status overlay initialize failed");
  TEST_ASSERT_TRUE(StatusOverlay::instance().isReady());

  StatusOverlay::instance().attachToScreen(ui_Main);
  StatusOverlay::instance().show();
  StatusOverlay::instance().setPosition(LV_ALIGN_TOP_RIGHT, -10, 10);
  ESP_LOGI("INTEGRATED_TEST", "Status overlay initialized and attached");
  BOOT_MEMORY_ANALYSIS("After status overlay...");

  // Initially hide all status icons until services are ready
  StatusOverlay::instance().updateWiFiStatus(WiFiState::None);
  StatusOverlay::instance().updateBLEStatus(false);
  StatusOverlay::instance().updateAudioStatus(AudioState::None);
  ESP_LOGI("INTEGRATED_TEST", "Status icons hidden initially");

  // Initialize BLE keyboard
  TEST_ASSERT_TRUE_MESSAGE(BLEKeyboard::instance().initialize(), "BLE keyboard initialize failed");
  ESP_LOGI("INTEGRATED_TEST", "BLE keyboard initialized, scanning for devices...");
  BOOT_MEMORY_ANALYSIS("After ble keyboard...");

  // Initialize network control
  TEST_ASSERT_TRUE_MESSAGE(NetworkControl::instance().initialize(), "Network control initialize failed");
  NetworkControl::instance().randomizeMACAddress();
  NetworkControl::instance().connectWithSavedCredentials();
  ESP_LOGI("INTEGRATED_TEST", "Network control initialized, attempting WiFi connection...");
  BOOT_MEMORY_ANALYSIS("After network control...");

  // Initialize audio manager
  TEST_ASSERT_TRUE_MESSAGE(AudioManager::instance().initialize(), "Audio manager initialize failed");
  ESP_LOGI("INTEGRATED_TEST", "Audio manager initialized");
  BOOT_MEMORY_ANALYSIS("After audio manager...");

  ESP_LOGI("INTEGRATED_TEST", "Setup completed successfully!");
}

void loop() {
  // Process display updates
  if (DisplayManager::instance().isReady()) {
    DisplayManager::instance().tick();
  }

  // Process status overlay updates
  if (StatusOverlay::instance().isReady()) {
    StatusOverlay::instance().tick();
  }

  // Process BLE keyboard
  if (BLEKeyboard::instance().isReady()) {
    BLEKeyboard::instance().tick();

    // Check BLE connection status and update UI
    bool bleConnected = BLEKeyboard::instance().isConnected();
    if (bleConnected != g_bleConnected) {
      g_bleConnected = bleConnected;
      if (StatusOverlay::instance().isReady()) {
        if (bleConnected) {
          StatusOverlay::instance().updateBLEStatus(true, "BLE Keyboard");
          ESP_LOGI("INTEGRATED_TEST", "BLE keyboard connected!");
          BOOT_MEMORY_ANALYSIS("After ble keyboard connected...");
        } else {
          StatusOverlay::instance().updateBLEStatus(false);
          ESP_LOGI("INTEGRATED_TEST", "BLE keyboard disconnected");
        }
      }
    }
    if (!bleConnected && !BLEKeyboard::instance().isScanning() && millis() - BLEKeyboard::instance().getScanEndTime() > 1000) {
      ESP_LOGI("INTEGRATED_TEST", "When not connected and scan ended more than 1 second ago, start scan again");
      BLEKeyboard::instance().startScan();
    }
  }

  // Process all events in the event system
  EventSystem::instance().processAllEvents();

  // Process network control
  if (NetworkControl::instance().isReady()) {
    NetworkControl::instance().tick();

    // Check WiFi connection status and update UI
    bool wifiConnected = NetworkControl::instance().isConnected();
    if (wifiConnected != g_wifiConnected) {
      g_wifiConnected = wifiConnected;
      if (StatusOverlay::instance().isReady()) {
        if (wifiConnected) {
          String ssid = WiFi.SSID();
          StatusOverlay::instance().updateWiFiStatus(WiFiState::Ready, ssid);
          ESP_LOGI("INTEGRATED_TEST", "WiFi connected to: %s", ssid.c_str());
          MainScreen::instance().onConnectionReady();
          BOOT_MEMORY_ANALYSIS("After wifi connected...");
        } else {
          StatusOverlay::instance().updateWiFiStatus(WiFiState::None);
          ESP_LOGI("INTEGRATED_TEST", "WiFi disconnected");
        }
      }
    }
    if (!wifiConnected && !NetworkControl::instance().isConnecting() && millis() - NetworkControl::instance().getConnectEndTime() > 10000) {
      if (!NetworkControl::instance().isOnSettingScreen()) {
        ESP_LOGI("INTEGRATED_TEST", "When not connected and connect ended more than 10 seconds ago, start connect again");
        NetworkControl::instance().randomizeMACAddress();
        NetworkControl::instance().connectWithTryingCredentials();
      }
    } else {
      static uint32_t lastCheck = 0;
      while (millis() - lastCheck > 1000) {
        lastCheck = millis();
        // ESP_LOGD("INTEGRATED_TEST", "Check why not hit reconnect: %d, %d, %d", wifiConnected, NetworkControl::instance().isConnecting(), millis() -
        // NetworkControl::instance().getConnectEndTime());
      }
    }
  }

  // Process audio manager
  if (AudioManager::instance().isReady()) {
    AudioManager::instance().tick();

    // Check audio readiness and update UI
    bool audioReady = AudioManager::instance().isReady();
    if (audioReady != g_audioReady) {
      g_audioReady = audioReady;
      if (StatusOverlay::instance().isReady()) {
        if (audioReady) {
          StatusOverlay::instance().updateAudioStatus(AudioState::Ready);
          ESP_LOGI("INTEGRATED_TEST", "Audio system ready");
          BOOT_MEMORY_ANALYSIS("After audio system ready...");
        } else {
          StatusOverlay::instance().updateAudioStatus(AudioState::None);
          ESP_LOGI("INTEGRATED_TEST", "Audio system not ready");
        }
      }
    }
  }

  // Process main screen
  if (MainScreen::instance().isReady()) {
    MainScreen::instance().tick();
  }

  delay(10); // Small delay to prevent overwhelming the system
}
