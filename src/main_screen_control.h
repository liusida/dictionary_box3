#ifndef MAIN_SCREEN_CONTROL_H
#define MAIN_SCREEN_CONTROL_H

#include "ui/ui.h"
#include <Arduino.h>

// Main screen control functions
void enterMainState();
void submitFormMainScreen();
void keyInMainScreen(char key);

// Audio read triggers
void readWord();
void readExplanation();
void readSampleSentence();
void playAudioFromServer(const char* word, const char* audioType);

// Fetch data from backend for a given word; returns true on success
bool getExplanationFromServer(const char* word,
                              String& outWord,
                              String& outExplanation,
                              String& outSampleSentence);

#endif // MAIN_SCREEN_CONTROL_H
