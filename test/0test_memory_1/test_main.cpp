#include <Arduino.h>
#include <unity.h>
#include "log.h"
#include "utils.h"
#include "memory_test_helper.h"
#include "AudioTools.h"
#include "AudioTools/CoreAudio/AudioHttp/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "audio_source_dynamic_url_no_auto_next.h"
#include "test_wifi_credentials.h"
#include "network_control.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

using namespace dict;

NetworkControl* g_network = nullptr;

void test_player() { // consumes less then 100KB of SRAM
  AudioPlayer *player;
  AudioSourceDynamicURLNoAutoNext* urlSource;
  AudioBoard board(AudioDriverES8311, NoPins);
  AudioBoardStream out(board);
  MP3DecoderHelix decoder;
  URLStream urlStream;

  urlSource = new AudioSourceDynamicURLNoAutoNext(urlStream, "audio/mp3");
  urlSource->addURL("https://dict.liusida.com/api/audio/stream?word=apple&type=explanation");
  ESP_LOGI("Test", "Creating player");
  player = new AudioPlayer(*urlSource, out, decoder);
  ESP_LOGI("Test", "Player begin");
  player->begin();
  // ESP_LOGI("Test", "Player copy");
  // player->copy();
  
  printMemoryStatus();

  ESP_LOGI("Test", "Player end");
  if (player) {
    player->end();
    delete player;
    player = nullptr;
  }
  if (urlSource) {
    delete urlSource;
    urlSource = nullptr;
  }
}

void test_client() { //only consumes a little memory
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  https.setReuse(false);
  
  if (!https.begin(client, "https://dict.liusida.com/api/define")) {
      ESP_LOGE("Test", "https.begin failed");
      return;
  }
  https.addHeader("Content-Type", "application/json");
  int httpCode = https.POST("{\"word\":\"apple\"}");
  https.end();
}

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
  g_network = new NetworkControl();
  TEST_ASSERT_TRUE_MESSAGE(g_network->initialize(), "Network control initialize failed");
  g_network->begin();
  g_network->connectToNetwork(TEST_WIFI_SSID, TEST_WIFI_PASSWORD);
  while (!g_network->isConnected()) {
    g_network->tick();
    delay(100);
  }
  ESP_LOGI("Test", "Starting test");
  printMemoryStatus();

  for (int i = 0; i < 5; i++) {
    test_client();
    printMemoryStatus();
  }


}

void loop() {}