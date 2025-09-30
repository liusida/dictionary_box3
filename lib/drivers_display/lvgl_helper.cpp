#include "lvgl_helper.h"
#include "core_eventing/event_system.h"
#include "core_eventing/events.h"
#include "core_misc/log.h"
#include "core_misc/utils.h"
#include "audio_manager.h" // can't use -I lib, should sort out later

namespace dict {

static const char *TAG = "LVGLHelper";

extern AudioManager* g_audio;

lv_group_t *getDefaultGroup() {
    lv_group_t *default_group = lv_group_get_default();
    if (default_group == nullptr) {
        default_group = lv_group_create();
        lv_group_set_default(default_group);
    }
    return default_group;
}

void addObjectToDefaultGroup(lv_obj_t *obj) {
    if (obj == nullptr) {
        return;
    }
    lv_group_t *default_group = getDefaultGroup();
    lv_group_add_obj(default_group, obj);
}

void loadScreen(lv_obj_t *screen) {
    lv_disp_load_scr(screen);
    lv_group_t *default_group = getDefaultGroup();
    lv_group_remove_all_objs(default_group);
    delay(100);
}


// --- Key handling from KeyEvent bus ---
static SubmitCallback s_onSubmit = nullptr;
static KeyInCallback s_onKeyIn = nullptr;
static EventBus<KeyEvent>::ListenerId s_keyListenerId = 0;
static FunctionKeyCallback s_onFunctionKeyIn = nullptr;
static EventBus<FunctionKeyEvent>::ListenerId s_functionKeyListenerId = 0;

static void handleKeyEvent(const KeyEvent& ev) {
    if (!ev.valid) return;
    char key = ev.key;
    lv_group_t *group = lv_group_get_default();
    lv_obj_t *focused = lv_group_get_focused(group);
    if (focused && lv_obj_has_class(focused, &lv_textarea_class)) {
        if (key == 0x08) {
            const char *text = lv_textarea_get_text(focused);
            if (text && strlen(text) > 0) {
                lv_textarea_delete_char(focused);
            }
        } else if (key == '\n') {
            lv_textarea_t *ta = (lv_textarea_t *)focused;
            if (ta->one_line) {
                if (s_onSubmit) s_onSubmit();
            } else {
                lv_textarea_add_char(focused, '\n');
            }
        } else if (key >= 32 && key <= 126) {
            lv_textarea_add_char(focused, key);
        }
    }
    if (key == 0x08 || (key >= 32 && key <= 126)) {
        if (s_onKeyIn) s_onKeyIn(key);
    }
}

static void handleFunctionKeyEvent(const FunctionKeyEvent& ev) {
    switch (ev.type) {
        case FunctionKeyEvent::PrintMemoryStatus:
            ESP_LOGI(TAG, "F1 pressed - printing memory status");
            printMemoryStatus(); // You'd need to implement this
            printAllStatus();
            break;
        case FunctionKeyEvent::VolumeDown:
            ESP_LOGI(TAG, "F10 pressed - volume down");
            if (g_audio) {
                g_audio->setVolume(g_audio->getVolume() - 0.05);
            }
            break;
        case FunctionKeyEvent::VolumeUp:
            ESP_LOGI(TAG, "F11 pressed - volume up");
            if (g_audio) {
                g_audio->setVolume(g_audio->getVolume() + 0.05);
            }
            break;
        case FunctionKeyEvent::WifiSettings:
        case FunctionKeyEvent::ReadWord:
        case FunctionKeyEvent::ReadExplanation:
        case FunctionKeyEvent::ReadSampleSentence:
        case FunctionKeyEvent::DownArrow:
        case FunctionKeyEvent::UpArrow:
        case FunctionKeyEvent::LeftArrow:
        case FunctionKeyEvent::RightArrow:
        case FunctionKeyEvent::Escape:
            ESP_LOGI(TAG, "Function key passed to callback");
            if (s_onFunctionKeyIn) s_onFunctionKeyIn(ev.type);
            break;
        default:
            break;
    }
}

void lvglEnableKeyEventHandler() {
    auto& bus = EventSystem::instance().getEventBus<KeyEvent>();
    s_keyListenerId = bus.subscribe(handleKeyEvent);
    
    auto& funcBus = EventSystem::instance().getEventBus<FunctionKeyEvent>();
    s_functionKeyListenerId = funcBus.subscribe(handleFunctionKeyEvent);
}

void lvglSetKeyCallbacks(const SubmitCallback& onSubmit, const KeyInCallback& onKeyIn) {
    s_onSubmit = onSubmit;
    s_onKeyIn = onKeyIn;
}

void lvglSetFunctionKeyCallbacks(const FunctionKeyCallback& onFunctionKeyIn) {
    s_onFunctionKeyIn = onFunctionKeyIn;
}

void lvglRemoveKeyEventHandler() {
    auto& bus = EventSystem::instance().getEventBus<KeyEvent>();
    bus.unsubscribe(s_keyListenerId);
    s_keyListenerId = 0;
    s_onSubmit = nullptr;
    s_onKeyIn = nullptr;
}

void lvglRemoveFunctionKeyEventHandler() {
    auto& funcBus = EventSystem::instance().getEventBus<FunctionKeyEvent>();
    funcBus.unsubscribe(s_functionKeyListenerId);
    s_functionKeyListenerId = 0;
    s_onFunctionKeyIn = nullptr;
}

} // namespace dict


