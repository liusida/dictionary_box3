#pragma once
#include "api_dictionary/dictionary_api.h"
#include "core_eventing/events.h"

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

private:
    bool initialized_;
    bool visible_;
    String currentWord_;
    DictionaryApi dictionaryApi_;
    DictionaryResult currentResult_;
};

} // namespace dict