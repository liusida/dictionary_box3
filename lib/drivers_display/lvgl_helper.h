#pragma once
#include "common.h"
#include "core_eventing/events.h"
#include "lvgl.h"
#include <functional>

namespace dict {

using SubmitCallback = std::function<void()>;
using KeyInCallback = std::function<void(char)>;
using FunctionKeyCallback = std::function<void(const FunctionKeyEvent &)>;

lv_group_t *getDefaultGroup();
void loadScreen(lv_obj_t *screen);
void addObjectToDefaultGroup(lv_obj_t *obj);

// Key handling via event bus
void lvglEnableKeyEventHandler();
void lvglSetKeyCallbacks(const SubmitCallback &onSubmit, const KeyInCallback &onKeyIn);
void lvglRemoveKeyEventHandler();

void lvglSetFunctionKeyCallbacks(const FunctionKeyCallback &onFunctionKeyIn);
void lvglRemoveFunctionKeyEventHandler();

} // namespace dict
