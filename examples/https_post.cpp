#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

// point to the file specified in platformio.ini
// certs/x509_crt_bundle
extern const uint8_t certs_x509_crt_bundle_start[] asm("_binary_certs_x509_crt_bundle_start");
extern const uint8_t certs_x509_crt_bundle_end[]   asm("_binary_certs_x509_crt_bundle_end");

static const char *SSID = "";
static const char *PASSWORD = "";
static const char *MP3_URL = "https://dict.liusida.com/apple.mp3";

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.setTxPower(WIFI_POWER_19_5dBm); // boost TX power

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

    WiFi.setDNS(IPAddress(8, 8, 8, 8), IPAddress(114, 114, 114, 114));

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = 0;
    while (now < 1700000000) {  // wait until time is set
        delay(200);
        time(&now);
    }

    const char *url = "https://dict.liusida.com/api/define";
    NetworkClientSecure client;
    client.setCACertBundle(certs_x509_crt_bundle_start, certs_x509_crt_bundle_end - certs_x509_crt_bundle_start);
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(client, url)) { // HTTPS
        Serial.print("[HTTPS] POST /lookup...\n");
        https.addHeader("Content-Type", "application/json");
        const char *word = "apple"; // TODO: set dynamically as needed
        String body = String("{\"word\":\"") + word + "\"}";
        int httpCode = https.POST(body);

        if (httpCode > 0) {
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                String payload = https.getString();
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
    } else {
        Serial.printf("[HTTPS] Unable to connect\n");
    }
}

void loop() {
    // no-op
}