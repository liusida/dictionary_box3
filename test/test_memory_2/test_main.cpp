#include <Arduino.h>
#include <unity.h>
#include "memory_test_helper.h"
#include "utils.h"
#include "event_system.h"
#include "event_publisher.h"
#include "events.h"

using namespace dict;

// Create a test to measure event system memory usage
void test_event_system_memory_usage() {
  uint32_t initialHeap = ESP.getFreeHeap();
  uint32_t initialPsram = ESP.getFreePsram();
  
  ESP_LOGI("EventTest", "=== BEFORE Event System Initialization ===");
  printMemoryStatus();
  
  // Initialize event system
  EventSystem& eventSystem = EventSystem::instance();
  
  ESP_LOGI("EventTest", "=== AFTER Event System Initialization ===");
  printMemoryStatus();
  
  // Create and publish various events to measure queue usage
  ESP_LOGI("EventTest", "=== Publishing Events ===");
  
  // Publish different event types
  EventPublisher::instance().publish(AudioEvent(AudioEvent::PlaybackStarted, "http://example.com/audio.mp3"));
  EventPublisher::instance().publish(WiFiEvent(WiFiEvent::Connected, "MyWiFi", IPAddress(192,168,1,100)));
  EventPublisher::instance().publish(DictionaryEvent(DictionaryEvent::LookupCompleted, "test", "explanation", "sample"));
  EventPublisher::instance().publish(UIEvent(UIEvent::ScreenChanged, "main", "input", "button"));
  
  ESP_LOGI("EventTest", "=== AFTER Publishing Events ===");
  printMemoryStatus();
  
  // Process events
  eventSystem.processAllEvents();
  
  ESP_LOGI("EventTest", "=== AFTER Processing Events ===");
  printMemoryStatus();
  
  // Calculate memory usage
  uint32_t finalHeap = ESP.getFreeHeap();
  uint32_t finalPsram = ESP.getFreePsram();
  
  int32_t heapUsed = initialHeap - finalHeap;
  int32_t psramUsed = initialPsram - finalPsram;
  
  ESP_LOGI("EventTest", "Event System Memory Usage:");
  ESP_LOGI("EventTest", "  Heap Used: %d bytes (%.2f KB)", heapUsed, heapUsed / 1024.0);
  ESP_LOGI("EventTest", "  PSRAM Used: %d bytes (%.2f KB)", psramUsed, psramUsed / 1024.0);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  ESP_LOGI("EventTest", "=== Event System Memory Usage Tests ===");
  setUpMemoryMonitoring();
  RUN_TEST(test_event_system_memory_usage);
  tearDownMemoryMonitoring("Event System Memory Usage");
}

void loop() {}