#include "lvgl_helper.h"
#include <Arduino.h>
#include "event_system.h"
#include "events.h"
#include "log.h"

namespace dict {

lv_group_t *getDefaultGroup() {
    lv_group_t *default_group = lv_group_get_default();
    if (default_group == nullptr) {
        default_group = lv_group_create();
        lv_group_set_default(default_group);
    }
    return default_group;
}

void loadScreen(lv_obj_t *screen) {
    lv_disp_load_scr(screen);
    lv_group_t *default_group = getDefaultGroup();
    lv_group_remove_all_objs(default_group);
    delay(100);
}

void addObjectToDefaultGroup(lv_obj_t *obj) {
    if (obj == nullptr) {
        return;
    }
    lv_group_t *default_group = getDefaultGroup();
    lv_group_add_obj(default_group, obj);
}

// --- Key handling from KeyEvent bus ---
static void (*s_onSubmit)() = nullptr;
static void (*s_onKeyIn)(char) = nullptr;
static core::eventing::EventBus<core::eventing::KeyEvent>::ListenerId s_keyListenerId = 0;

static void handleKeyEvent(const core::eventing::KeyEvent& ev) {
    if (!ev.valid) return;
    char key = ev.key;
    lv_group_t *group = lv_group_get_default();
    lv_obj_t *focused = lv_group_get_focused(group);
    if (focused && lv_obj_has_class(focused, &lv_textarea_class)) {
        if (key == 0x08) {
            lv_textarea_delete_char(focused);
            if (s_onKeyIn) s_onKeyIn(key);
        } else if (key == '\n') {
            lv_textarea_t *ta = (lv_textarea_t *)focused;
            if (ta->one_line) {
                if (s_onSubmit) s_onSubmit();
            } else {
                lv_textarea_add_char(focused, '\n');
            }
        } else if (key >= 32 && key <= 126) {
            lv_textarea_add_char(focused, key);
            if (s_onKeyIn) s_onKeyIn(key);
        }
    }
}

void lvglInstallKeyEventHandler(void (*onSubmit)(), void (*onKeyIn)(char)) {
    s_onSubmit = onSubmit;
    s_onKeyIn = onKeyIn;
    auto& bus = core::eventing::EventSystem::instance().getEventBus<core::eventing::KeyEvent>();
    s_keyListenerId = bus.subscribe(handleKeyEvent);
}

void lvglSetKeyCallbacks(void (*onSubmit)(), void (*onKeyIn)(char)) {
    s_onSubmit = onSubmit;
    s_onKeyIn = onKeyIn;
}

void lvglRemoveKeyEventHandler() {
    auto& bus = EventSystem::instance().getEventBus<KeyEvent>();
    bus.unsubscribe(s_keyListenerId);
    s_keyListenerId = 0;
    s_onSubmit = nullptr;
    s_onKeyIn = nullptr;
}

} // namespace dict


