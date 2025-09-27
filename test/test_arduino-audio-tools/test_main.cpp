/**
 * @file player-url-i2s.ino
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-url-i2s/README.md
 *
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
#include "test_wifi_credentials.h"
#define PIN_I2S_WS I2S_WS
#define PIN_I2S_BCK I2S_SCLK
#define PIN_I2S_DATA_OUT I2S_DOUT
#define PIN_I2S_DATA_IN I2S_SDIN
#define PIN_I2S_MCK I2S_MCLK

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/Disk/AudioSourceURL.h"
#include "log.h"
#include "utils.h"

using namespace dict;

const char *urls[] = {"https://dict.liusida.com/api/audio/stream?word=apple&type=word"};
const char *wifi = TEST_WIFI_SSID;
const char *password = TEST_WIFI_PASSWORD;

URLStream urlStream(wifi, password);
AudioSourceURL source(urlStream, urls, "audio/mp3");
I2SStream i2s;
MP3DecoderHelix decoder;
AudioPlayer player(source, i2s, decoder);

void setup() {
    Serial.begin(115200);
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
    printMemoryStatus();

    // setup output
    auto cfg = i2s.defaultConfig(TX_MODE);
    printMemoryStatus();
    i2s.begin(cfg);
    printMemoryStatus();

    // setup player
    player.begin();
    printMemoryStatus();
}

void loop() {
    static bool isRunning = true;
    static uint32_t inc = 0;
    if (isRunning) {
        inc++;
        player.copy();
        delay(10);
        static uint32_t lastShowTime = millis();
        if (millis() - lastShowTime > 1000) {
            lastShowTime = millis();
            printMemoryStatus();
            ESP_LOGI("Test", "inc: %d", inc);
        }

        if (millis() > 10000) {
            ESP_LOGI("Test", "10 seconds passed, exiting");
            isRunning = false;
            printMemoryStatus();
            player.stop();
            printMemoryStatus();
            player.end();
            printMemoryStatus();
            urlStream.end();
            printMemoryStatus();
            i2s.end();
            printMemoryStatus();
        }
    }
}