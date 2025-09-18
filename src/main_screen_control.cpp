#include "main_screen_control.h"
#include "main.h"
#include "drivers/drivers.h"
#include "utils.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "esp_log.h"

extern AudioManager audio;

static const char *TAG = "MainScreen";

static const char *LOOKUP_URL = "https://dict.liusida.com/api/define";
static const char *AUDIO_URL = "https://dict.liusida.com/api/audio/stream";

static inline void playCurrentWordAudio(const char* type) {
  String word = lv_label_get_text(ui_TxtWord);
  playAudioFromServer(word.c_str(), type);
}

void readWord() {
  playCurrentWordAudio("word");
}

void readExplanation() {
  playCurrentWordAudio("explanation");
}

void readSampleSentence() {
  playCurrentWordAudio("sample_sentence");
}

// Minimal URL encoder for query parameters
static String urlEncode(const char *s) {
  String out;
  if (s == nullptr) return out;
  while (*s) {
    char c = *s++;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      out += c;
    } else if (c == ' ') {
      out += '%'; out += '2'; out += '0';
    } else {
      char buf[4];
      static const char *hex = "0123456789ABCDEF";
      buf[0] = '%';
      buf[1] = hex[(static_cast<unsigned char>(c) >> 4) & 0x0F];
      buf[2] = hex[static_cast<unsigned char>(c) & 0x0F];
      buf[3] = 0;
      out += buf;
    }
  }
  return out;
}

void playAudioFromServer(const char* word, const char* audioType) {
  if (word == nullptr) return;
  // Skip if word is empty or literal "null"
  if (word[0] == '\0') return;
  if (strcasecmp(word, "null") == 0) return;
  if (WiFi.status() != WL_CONNECTED) {
    ESP_LOGW(TAG, "WiFi not connected; skip audio");
    return;
  }
  String encoded = urlEncode(word);
  if (encoded.length() == 0) return;
  String url = String(AUDIO_URL) + "?word=" + encoded + "&type=" + audioType;
  ESP_LOGD(TAG, "Playing audio from server: %s", url.c_str());
  audio.play(url.c_str());
}

bool getExplanationFromServer(const char* word,
                              String& outWord,
                              String& outExplanation,
                              String& outSampleSentence) {
  if (WiFi.status() != WL_CONNECTED) {
    ESP_LOGW(TAG, "WiFi not connected");
    return false;
  }

  // Build JSON body
  JsonDocument doc;
  doc["word"] = word;
  String body;
  serializeJson(doc, body);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (!https.begin(client, LOOKUP_URL)) {
    ESP_LOGE(TAG, "https.begin failed");
    return false;
  }

  https.addHeader("Content-Type", "application/json");
  int httpCode = https.POST(body);
  if (httpCode <= 0) {
    ESP_LOGE(TAG, "POST failed: %s", https.errorToString(httpCode).c_str());
    https.end();
    return false;
  }

  if (httpCode != HTTP_CODE_OK) {
    ESP_LOGW(TAG, "HTTP %d", httpCode);
    https.end();
    return false;
  }

  String payload = https.getString();
  https.end();

  JsonDocument resp;
  DeserializationError err = deserializeJson(resp, payload);
  if (err) {
    ESP_LOGE(TAG, "JSON parse error: %s", err.c_str());
    return false;
  }

  JsonVariant vWord = resp["word"];
  JsonVariant vExpl = resp["explanation"];
  JsonVariant vSample = resp["sample_sentence"];

  outWord = vWord.isNull() ? String("") : vWord.as<String>();
  outExplanation = vExpl.isNull() ? String("") : vExpl.as<String>();
  outSampleSentence = vSample.isNull() ? String("") : vSample.as<String>();
  if (outSampleSentence.equalsIgnoreCase("null")) outSampleSentence = "";
  if (outExplanation.equalsIgnoreCase("null")) outExplanation = "";
  if (outWord.equalsIgnoreCase("null")) outWord = "";

  // Fallbacks for sample sentence under alternate JSON shapes
  if (outSampleSentence.length() == 0) {
    if (resp["sampleSentence"].is<const char*>()) {
      outSampleSentence = resp["sampleSentence"].as<String>();
    } else if (resp["sample"].is<const char*>()) {
      outSampleSentence = resp["sample"].as<String>();
    } else if (resp["sentence"].is<const char*>()) {
      outSampleSentence = resp["sentence"].as<String>();
    } else if (resp["example"].is<const char*>()) {
      outSampleSentence = resp["example"].as<String>();
    } else if (resp["examples"].is<JsonArray>() && resp["examples"].size() > 0) {
      outSampleSentence = resp["examples"][0].as<String>();
    } else if (resp["samples"].is<JsonArray>() && resp["samples"].size() > 0) {
      outSampleSentence = resp["samples"][0].as<String>();
    }
    if (outSampleSentence.equalsIgnoreCase("null")) outSampleSentence = "";
  }

  ESP_LOGD(TAG, "Parsed JSON -> word len: %d, expl len: %d, sample len: %d",
           outWord.length(), outExplanation.length(), outSampleSentence.length());

  // If sample is still empty, dump a truncated view of the raw payload to debug mapping
  if (outSampleSentence.length() == 0) {
    const int total = payload.length();
    const int to_show = total > 256 ? 256 : total;
    ESP_LOGD(TAG, "Raw payload (first %d of %d): %.*s", to_show, total, to_show, payload.c_str());
  }
  return outWord.length() > 0;
}

void submitFormMainScreen() {
  if (strlen(lv_textarea_get_text(ui_InputWord)) == 0) {
    return;
  }
  ESP_LOGI(TAG, "Submitting form");
  lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(ui_TxtWord, lv_textarea_get_text(ui_InputWord));
  lv_textarea_set_text(ui_InputWord, "");
  // Fetch data
  String inWord = lv_label_get_text(ui_TxtWord);
  String outWord, outExpl, outSample;
  if (getExplanationFromServer(inWord.c_str(), outWord, outExpl, outSample)) {
    lv_label_set_text(ui_TxtWord, outWord.c_str());
    lv_label_set_text(ui_TxtExplanation, outExpl.c_str());
    lv_label_set_text(ui_TxtSampleSentence, outSample.c_str());
  } else {
    lv_label_set_text(ui_TxtExplanation, "Request failed");
  }
}

void keyInMainScreen(char key) {
  ESP_LOGD(TAG, "Key in main screen");
  lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
}

void enterMainState() {
    ESP_LOGI(TAG, "Entering MAIN state");
    currentState = STATE_MAIN;
    stateTransitioned = true;
    loadScreen(ui_Main);

    addObjectToDefaultGroup(ui_InputWord);
    lv_textarea_set_text(ui_InputWord, "");
    lv_group_focus_obj(ui_InputWord);
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_TxtWord, "");
    lv_label_set_text(ui_TxtExplanation, "");
    lv_label_set_text(ui_TxtSampleSentence, "");

    setSubmitCallback(submitFormMainScreen);
    setKeyInCallback(keyInMainScreen);
}
