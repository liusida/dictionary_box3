#pragma once

#include "lvgl.h"
#include "events.h"

namespace dict {

lv_group_t *getDefaultGroup();
void loadScreen(lv_obj_t *screen);
void addObjectToDefaultGroup(lv_obj_t *obj);

// Key handling via event bus
void lvglInstallKeyEventHandler(void (*onSubmit)() = nullptr, void (*onKeyIn)(char) = nullptr);
void lvglSetKeyCallbacks(void (*onSubmit)(), void (*onKeyIn)(char));
void lvglRemoveKeyEventHandler();

} // namespace dict


