#include "dictionary_service.h"
#include "core/services.h"
#include "core/event_publisher.h"
#include "drivers/audio_manager.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "core/log.h"

static const char *TAG = "DictionaryService";

DictionaryService::DictionaryService() 
    : baseUrl_("https://dict.liusida.com/api/define"),
      audioUrl_("https://dict.liusida.com/api/audio/stream"),
      initialized_(false) {
}

DictionaryService::~DictionaryService() {
    shutdown();
}

bool DictionaryService::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing dictionary service...");
    initialized_ = true;
    ESP_LOGI(TAG, "Dictionary service initialized");
    return true;
}

void DictionaryService::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down dictionary service...");
    initialized_ = false;
}

DictionaryResult DictionaryService::lookupWord(const String& word) {
    if (!isReady()) {
        ESP_LOGW(TAG, "Service not ready (WiFi not connected)");
        return DictionaryResult();
    }
    
    if (!isWordValid(word)) {
        ESP_LOGW(TAG, "Invalid word provided");
        return DictionaryResult();
    }
    
    ESP_LOGI(TAG, "Looking up word: %s", word.c_str());
    
    // Publish lookup started event
    EventPublisher::instance().publishDictionaryEvent(DictionaryEvent::LookupStarted, word);
    
    // Build JSON body
    JsonDocument doc;
    doc["word"] = word;
    String body;
    serializeJson(doc, body);
    
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    
    if (!https.begin(client, baseUrl_.c_str())) {
        ESP_LOGE(TAG, "https.begin failed");
        return DictionaryResult();
    }
    
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(body);
    
    if (httpCode <= 0) {
        ESP_LOGE(TAG, "POST failed: %s", https.errorToString(httpCode).c_str());
        https.end();
        return DictionaryResult();
    }
    
    if (httpCode != HTTP_CODE_OK) {
        ESP_LOGW(TAG, "HTTP %d", httpCode);
        https.end();
        return DictionaryResult();
    }
    
    String payload = https.getString();
    https.end();
    
    JsonDocument resp;
    DeserializationError err = deserializeJson(resp, payload);
    if (err) {
        ESP_LOGE(TAG, "JSON parse error: %s", err.c_str());
        return DictionaryResult();
    }
    
    String outWord = resp["word"].isNull() ? String("") : resp["word"].as<String>();
    String outExplanation = resp["explanation"].isNull() ? String("") : resp["explanation"].as<String>();
    String outSampleSentence = resp["sample_sentence"].isNull() ? String("") : resp["sample_sentence"].as<String>();
    
    // Handle null values
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
    
    bool success = outWord.length() > 0;
    
    // Publish lookup completed event
    if (success) {
        EventPublisher::instance().publishDictionaryEvent(DictionaryEvent::LookupCompleted, outWord, outExplanation, outSampleSentence);
    } else {
        EventPublisher::instance().publishDictionaryEvent(DictionaryEvent::LookupFailed, word, "", "", "No results found");
    }
    
    return DictionaryResult(outWord, outExplanation, outSampleSentence, success);
}

bool DictionaryService::playAudio(const String& word, const String& audioType) {
    if (!isReady()) {
        ESP_LOGW(TAG, "Service not ready (WiFi not connected)");
        return false;
    }
    
    if (!isWordValid(word)) {
        ESP_LOGW(TAG, "Invalid word for audio playback");
        return false;
    }
    
    String encoded = urlEncode(word);
    if (encoded.length() == 0) {
        return false;
    }
    
    String url = audioUrl_ + "?word=" + encoded + "&type=" + audioType;
    ESP_LOGD(TAG, "Playing audio from server: %s", url.c_str());
    
    // Publish audio requested event
    EventPublisher::instance().publishDictionaryEvent(DictionaryEvent::AudioRequested, word, "", "", audioType);
    
    return Services::instance().audio().play(url.c_str());
}

bool DictionaryService::isReady() const {
    return initialized_ && WiFi.status() == WL_CONNECTED;
}

void DictionaryService::setBaseUrl(const String& url) {
    baseUrl_ = url;
}

String DictionaryService::getBaseUrl() const {
    return baseUrl_;
}

String DictionaryService::urlEncode(const String& str) {
    String out;
    if (str.length() == 0) return out;
    
    for (size_t i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || 
            c == '-' || c == '_' || c == '.' || c == '~') {
            out += c;
        } else if (c == ' ') {
            out += "%20";
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

bool DictionaryService::isWordValid(const String& word) {
    if (word.length() == 0) return false;
    if (word.equalsIgnoreCase("null")) return false;
    return true;
}
