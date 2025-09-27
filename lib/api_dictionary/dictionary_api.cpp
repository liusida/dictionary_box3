#include "dictionary_api.h"
#include "core_eventing/event_publisher.h"
#include "core_misc/log.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

namespace dict {

static const char *TAG = "DictionaryApi";

DictionaryApi::DictionaryApi() 
    : hostname_("dict.liusida.com"),
      baseUrl_("https://dict.liusida.com/api/define"),
      audioBaseUrl_("https://dict.liusida.com/api/audio/stream"),
      initialized_(false),
      prewarmTaskHandle_(nullptr) {
}

DictionaryApi::~DictionaryApi() {
    shutdown();
    
    // Clean up prewarm task if it exists
    if (prewarmTaskHandle_ != nullptr) {
        vTaskDelete(prewarmTaskHandle_);
        prewarmTaskHandle_ = nullptr;
    }
}

bool DictionaryApi::initialize() {
    if (initialized_) {
        ESP_LOGI(TAG, "Dictionary API already initialized");
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing dictionary API client...");
    initialized_ = true;
    ESP_LOGI(TAG, "Dictionary API client initialized");
    return true;
}

void DictionaryApi::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down dictionary API client...");
    initialized_ = false;
}

DictionaryResult DictionaryApi::lookupWord(const String& word) {
    if (!isReady()) {
        ESP_LOGW(TAG, "Service not ready (WiFi not connected)");
        return DictionaryResult();
    }
    
    if (!isWordValid(word)) {
        ESP_LOGW(TAG, "Invalid word provided");

        EventPublisher::instance().publish(DictionaryEvent(DictionaryEvent::LookupFailed, word, "", "", "Invalid word"));
        return DictionaryResult();
    }
    
    ESP_LOGI(TAG, "Looking up word: %s", word.c_str());
    
    // Publish lookup started event
    EventPublisher::instance().publish(DictionaryEvent(DictionaryEvent::LookupStarted, word));
    
    // Build JSON body
    JsonDocument* doc = (JsonDocument*)ps_malloc(sizeof(JsonDocument)); //save some SRAM by using ps_malloc
    if (!doc) {
        ESP_LOGE(TAG, "Failed to allocate JSON doc in PSRAM");
        return DictionaryResult();
    }
    new(doc) JsonDocument();
    (*doc)["word"] = word;
    String body;
    serializeJson(*doc, body);
    doc->~JsonDocument();
    free(doc);
    
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    https.setReuse(false);
    
    if (!https.begin(client, baseUrl_.c_str())) {
        ESP_LOGE(TAG, "https.begin failed");
        return DictionaryResult();
    }
    
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(body);
    
    if (httpCode <= 0) {
        ESP_LOGE(TAG, "POST failed (1): %s", https.errorToString(httpCode).c_str());
        https.end();

        WiFiClientSecure client1;
        client1.setInsecure();
        if (!https.begin(client1, baseUrl_.c_str())) {
            ESP_LOGE(TAG, "https.begin failed");
            return DictionaryResult();
        }
        
        https.addHeader("Content-Type", "application/json");
        httpCode = https.POST(body);
        if (httpCode <= 0) {
            ESP_LOGE(TAG, "POST failed (2): %s", https.errorToString(httpCode).c_str());
            https.end();
            return DictionaryResult();
        }
    }
    
    if (httpCode != HTTP_CODE_OK) {
        ESP_LOGW(TAG, "HTTP %d", httpCode);
        https.end();
        return DictionaryResult();
    }
    
    String payload = https.getString();
    https.end();
    
    ESP_LOGD(TAG, "Payload: %s", payload.c_str());
    if (payload.length() == 0) {
        ESP_LOGE(TAG, "Payload is empty");
        return DictionaryResult();
    }

    JsonDocument* resp = (JsonDocument*)ps_malloc(sizeof(JsonDocument));
    if (!resp) {
        ESP_LOGE(TAG, "Failed to allocate response JSON in PSRAM");
        return DictionaryResult();
    }
    new(resp) JsonDocument();
    DeserializationError err = deserializeJson(*resp, payload);
    if (err) {
        ESP_LOGE(TAG, "JSON parse error: %s", err.c_str());
        free(resp);
        return DictionaryResult();
    }
    
    String outWord = (*resp)["word"].isNull() ? String("") : (*resp)["word"].as<String>();
    String outExplanation = (*resp)["explanation"].isNull() ? String("") : (*resp)["explanation"].as<String>();
    String outSampleSentence = (*resp)["sample_sentence"].isNull() ? String("") : (*resp)["sample_sentence"].as<String>();
    
    // Handle null values
    if (outSampleSentence.equalsIgnoreCase("null")) outSampleSentence = "";
    if (outExplanation.equalsIgnoreCase("null")) outExplanation = "";
    if (outWord.equalsIgnoreCase("null")) outWord = "";
    
    // Fallbacks for sample sentence under alternate JSON shapes
    if (outSampleSentence.length() == 0) {
        if ((*resp)["sampleSentence"].is<const char*>()) {
            outSampleSentence = (*resp)["sampleSentence"].as<String>();
        } else if ((*resp)["sample"].is<const char*>()) {
            outSampleSentence = (*resp)["sample"].as<String>();
        } else if ((*resp)["sentence"].is<const char*>()) {
            outSampleSentence = (*resp)["sentence"].as<String>();
        } else if ((*resp)["example"].is<const char*>()) {
            outSampleSentence = (*resp)["example"].as<String>();
        } else if ((*resp)["examples"].is<JsonArray>() && (*resp)["examples"].size() > 0) {
            outSampleSentence = (*resp)["examples"][0].as<String>();
        } else if ((*resp)["samples"].is<JsonArray>() && (*resp)["samples"].size() > 0) {
            outSampleSentence = (*resp)["samples"][0].as<String>();
        }
        if (outSampleSentence.equalsIgnoreCase("null")) outSampleSentence = "";
    }
    resp->~JsonDocument();
    free(resp);
    
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
        EventPublisher::instance().publish(DictionaryEvent(DictionaryEvent::LookupCompleted, outWord, outExplanation, outSampleSentence));
    } else {
        EventPublisher::instance().publish(DictionaryEvent(DictionaryEvent::LookupFailed, word, "", "", "No results found"));
    }
    
    return DictionaryResult(outWord, outExplanation, outSampleSentence, success);
}

AudioUrl DictionaryApi::getAudioUrl(const String& word, const String& audioType) {
    if (!isReady()) {
        ESP_LOGW(TAG, "Service not ready (WiFi not connected)");
        return AudioUrl();
    }
    
    if (!isWordValid(word)) {
        ESP_LOGW(TAG, "Invalid word for audio URL generation");
        return AudioUrl();
    }
    
    String encodedWord = urlEncode(word);
    if (encodedWord.length() == 0) {
        return AudioUrl();
    }

    String encodedAudioType = urlEncode(audioType);
    if (encodedAudioType.length() == 0) {
        return AudioUrl();
    }
    if (encodedAudioType.equalsIgnoreCase("sample_sentence")) {
        encodedAudioType = "sample";
    }
    
    String url = audioBaseUrl_ + "?word=" + encodedWord + "&type=" + encodedAudioType;
    ESP_LOGD(TAG, "Generated audio URL: %s", url.c_str());
    
    // Publish audio requested event
    EventPublisher::instance().publish(DictionaryEvent(DictionaryEvent::AudioRequested, word, "", "", audioType));
    
    return AudioUrl(url, audioType, true);
}

bool DictionaryApi::isReady() const {
    return initialized_ && WiFi.status() == WL_CONNECTED;
}

void DictionaryApi::setBaseUrl(const String& url) {
    baseUrl_ = url;
}

String DictionaryApi::getBaseUrl() const {
    return baseUrl_;
}

void DictionaryApi::setAudioBaseUrl(const String& url) {
    audioBaseUrl_ = url;
}

String DictionaryApi::getAudioBaseUrl() const {
    return audioBaseUrl_;
}

String DictionaryApi::urlEncode(const String& str) {
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

void DictionaryApi::prewarm() {
    // If task is already running, don't create another one
    if (prewarmTaskHandle_ != nullptr) {
        ESP_LOGW(TAG, "Prewarm task already running");
        return;
    }
    
    // Create async task for prewarm operation
    BaseType_t result = xTaskCreatePinnedToCore(
        prewarmTask,           // Task function
        "prewarm_task",        // Task name
        4096,                  // Stack size
        this,                  // Parameter (this instance)
        1,                     // Priority (low priority)
        &prewarmTaskHandle_,   // Task handle
        0                      // Core (0 or 1)
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create prewarm task");
        prewarmTaskHandle_ = nullptr;
    } else {
        ESP_LOGI(TAG, "Prewarm task created successfully");
    }
}

void DictionaryApi::prewarmTask(void* parameter) {
    DictionaryApi* api = static_cast<DictionaryApi*>(parameter);
    
    ESP_LOGI(TAG, "Starting async prewarm operation");
    
    // Perform the actual prewarm connection
    WiFiClientSecure client;
    client.setInsecure();
    
    if (client.connect(api->hostname_.c_str(), 443)) {
        ESP_LOGI(TAG, "Prewarm connection successful");
        client.stop();
    } else {
        ESP_LOGW(TAG, "Prewarm connection failed");
    }
    
    ESP_LOGI(TAG, "Async prewarm operation completed");
    
    // Clean up task handle and delete self
    api->prewarmTaskHandle_ = nullptr;
    vTaskDelete(nullptr);
}

bool DictionaryApi::isWordValid(const String& word) {
    if (word.length() == 0) return false;
    if (word.equalsIgnoreCase("null")) return false;
    return true;
}

} // namespace dict