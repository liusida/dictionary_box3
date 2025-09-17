#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "AudioTools/CoreAudio/AudioHttp/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

static const char* SSID     = "";
static const char* PASSWORD = "";
static const char* MP3_URL  = "http://192.168.1.164:1234/output.mp3";

// Audio board and output (ES8311)
AudioBoard board{AudioDriverES8311, NoPins};
AudioBoardStream out(board);
AudioInfo info(32000, 2, 16);

// Network stream and MP3 decoder pipeline
URLStream url;                    // HTTP client stream (uses Arduino WiFi)
MP3DecoderHelix mp3;              // MP3 decoder
EncodedAudioStream decoded(&out, &mp3); // writes decoded PCM to I2S output
StreamCopy copier(decoded, url, 4096);

// Fix for removing ID3v2 header
auto skipID3v2 = [&](Stream& s){
  uint8_t hdr[10];
  if (s.readBytes(hdr, 10) != 10) return;                 // not enough -> ignore
  if (hdr[0]=='I' && hdr[1]=='D' && hdr[2]=='3') {
    uint32_t sz = ((hdr[6]&0x7F)<<21)|((hdr[7]&0x7F)<<14)|((hdr[8]&0x7F)<<7)|(hdr[9]&0x7F);
    uint32_t toSkip = sz;                                  // header already consumed
    uint8_t buf[256];
    while (toSkip) {
      size_t n = s.readBytes(buf, toSkip > sizeof(buf) ? sizeof(buf) : toSkip);
      if (!n) break;
      toSkip -= n;
    }
  } else {
    // Not ID3v2: we consumed 10 bytes; simplest workaround is to re-open the URL
    // so we don't lose them:
    url.end();
    url.begin(MP3_URL, "audio/mpeg");
  }
};

void audioTask(void*){
  for(;;){
    copier.copy();
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);   // boost TX power

  Serial.printf("Connecting to %s\n", SSID);
  WiFi.begin(SSID, PASSWORD);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connect timeout");
    return;
  }

  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
  
  // Configure audio pins for ESP32-S3 Box 3
  Serial.println("initializing audio pins...");
  DriverPins pins;
  pins.addI2C(PinFunction::CODEC, 18, 8);           // SCL=18, SDA=8
  pins.addI2S(PinFunction::CODEC, 2, 17, 45, 15);   // MCLK=2, BCLK=17, WS=45, DOUT=15
  pins.addPin(PinFunction::PA, PA_PIN, PinLogic::Output);
  board.setPins(pins);

  // Start audio output
  Serial.println("starting I2S & codec...");
  out.begin();
  out.setAudioInfo(info);
  out.setVolume(0.7f);

  // Start network stream and decoder pipeline
  Serial.println("opening URL stream...");
  url.setTimeout(10000);
  if (!url.begin(MP3_URL, "audio/mpeg")) {
    Serial.println("URL open failed");
    return;
  }
  skipID3v2(url);

  // Initialize encoded stream so decoder can push PCM to output
  decoded.begin();
  copier.begin();
  // Avoid overrunning target by honoring availableForWrite()
  copier.setCheckAvailableForWrite(true);
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 10, nullptr, 0);
}

void loop() {
  // no-op
}