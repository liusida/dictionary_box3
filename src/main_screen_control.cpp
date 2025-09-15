#include "main_screen_control.h"
#include "main.h"
#include "drivers/drivers.h"
#include "utils.h"
#include <Arduino.h>

void submitFormMainScreen() {
  if (strlen(lv_textarea_get_text(ui_InputWord)) == 0) {
    return;
  }
  Serial.println("[MainScreen] Submitting form");
  lv_obj_add_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(ui_TxtWord, lv_textarea_get_text(ui_InputWord));
  lv_textarea_set_text(ui_InputWord, "");
  //TODO: connect to api to get explanation and sample sentence
}

void keyInMainScreen(char key) {
  Serial.println("[MainScreen] Key in main screen");
  lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
}

void enterMainState() {
    Serial.println("[MainScreen] Entering MAIN state");
    currentState = STATE_MAIN;
    stateTransitioned = true;
    loadScreen(ui_Main);

    addObjectToDefaultGroup(ui_InputWord);
    lv_textarea_set_text(ui_InputWord, "");
    lv_group_focus_obj(ui_InputWord);
    lv_obj_remove_flag(ui_InputWord, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(ui_TxtWord, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_TxtWord, "");
    lv_label_set_text(ui_TxtExplanation, "");
    lv_label_set_text(ui_TxtSampleSentence, "");

    setSubmitCallback(submitFormMainScreen);
    setKeyInCallback(keyInMainScreen);
}
