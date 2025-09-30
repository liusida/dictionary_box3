#pragma once
#include "api_dictionary/dictionary_api.h"
#include "core_eventing/events.h"
#include "wifi_settings_screen.h"

namespace dict {

class MainScreen {
public:
    MainScreen();
    ~MainScreen();
    
    // Core lifecycle methods
    bool initialize();
    void shutdown();
    void tick();
    bool isReady() const;
    
    // Screen control methods
    void show();
    void hide();
    bool isVisible() const;

    void onSubmit();
    void onKeyIn(char key);
    void onFunctionKeyEvent(const FunctionKeyEvent& event);
    void onConnectionReady();
    void onWifiSettings();
    void onBackFromWifiSettings();
    void onPlayAudio(const String& audioType);
    void onDownArrow();
    void onUpArrow();
    void onEscape();
    
private:
    bool initialized_;
    bool visible_;
    bool isScreenActive_;
    String currentWord_;
    DictionaryApi dictionaryApi_;
    DictionaryResult currentResult_;
    bool isWifiSettings_;
    WiFiSettingsScreen wifiSettingsScreen_;
};

} // namespace dict