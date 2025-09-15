#ifndef MAIN_H
#define MAIN_H

// Application states
enum AppState { STATE_SPLASH, STATE_MAIN };

// External state variables (defined in main.cpp)
extern AppState currentState;
extern bool stateTransitioned;

#endif // MAIN_H
