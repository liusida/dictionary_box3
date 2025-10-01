#include <Arduino.h>
#include <unity.h>
#include "main_app.h"
#include "key_processor.h"
#include "lvgl_helper.h"
#include "core_eventing/event_system.h"

using namespace dict;

class KeyboardInputSimulator {
 public:
  void type(const char* s) {
    for (const char* p=s; *p; ++p) publishKey(*p);
  }
  void enter() { publishKey('\n', 0x28, 0); }
  void backspace() { publishKey(0x08, 0x2A, 0); }
  void simulateFunctionKey(FunctionKeyEvent::Type type) {
    FunctionKeyEvent event(type);
    EventSystem::instance().getEventBus<FunctionKeyEvent>().publish(event);
  }
 private:
  void publishKey(char ch, uint8_t code=0, uint8_t mod=0) {
    KeyProcessor kp; 
    kp.initialize();
    kp.sendKeyToLVGL(ch, code, mod);
  }
};

static KeyboardInputSimulator sim;
static bool ran = false;

void setup() {
  Serial.begin(115200);
  delay(2000);
  main_setup();
}

void loop() {
  main_loop();
  EventSystem::instance().processAllEvents();
  
  if (!ran && millis() > 5000) {
    ESP_LOGI("KEYBOARD_SIM", "Starting keyboard simulation tests...");
    
    // Test 1: Normal word input
    ESP_LOGI("KEYBOARD_SIM", "Test 1: Normal word input");
    sim.type("hello");
    sim.enter();
    delay(1000);
    
    // Test 2: Rapid input
    ESP_LOGI("KEYBOARD_SIM", "Test 2: Rapid input");
    sim.type("abc");
    for (int i=0;i<3;i++) {
      sim.backspace();
      delay(100);
    }
    delay(1000);
    
    // Test 3: Long text
    ESP_LOGI("KEYBOARD_SIM", "Test 3: Long text input");
    sim.type("supercalifragilisticexpialidocious");
    sim.enter();
    delay(1000);
    
    // Test 4: Special characters
    ESP_LOGI("KEYBOARD_SIM", "Test 4: Special characters");
    sim.type("!@#$%^&*()");
    delay(1000);
    
    // Test 5: Function keys
    ESP_LOGI("KEYBOARD_SIM", "Test 5: Function keys");
    sim.simulateFunctionKey(FunctionKeyEvent::VolumeUp);
    delay(500);
    sim.simulateFunctionKey(FunctionKeyEvent::VolumeDown);
    delay(500);
    sim.simulateFunctionKey(FunctionKeyEvent::PrintMemoryStatus);
    delay(1000);
    
    // Test 6: Mixed operations
    ESP_LOGI("KEYBOARD_SIM", "Test 6: Mixed operations");
    sim.type("mixed");
    sim.backspace();
    sim.backspace();
    sim.type("ed");
    sim.enter();
    delay(1000);
    
    ESP_LOGI("KEYBOARD_SIM", "Keyboard simulation tests completed!");
    ran = true;
  }
  
  delay(10);
}
