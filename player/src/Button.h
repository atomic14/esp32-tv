#include <Arduino.h>

bool buttonRight(){
#ifdef BUTTON_R
  return (digitalRead(BUTTON_R) == 0);
#endif
  return false;
}

bool buttonLeft(){
#ifdef BUTTON_L
  return (digitalRead(BUTTON_L) == 0);
#endif
  return false;
}

bool buttonPowerOff() {
#ifdef BUTTON_L
  #ifdef BUTTON_R
    return (digitalRead(BUTTON_L) == 0 && digitalRead(BUTTON_R) == 0);
  #endif
#endif
  return false;
}

void buttonInit(){
#ifdef BUTTON_L
  #ifdef BUTTON_R
  pinMode(BUTTON_L, INPUT_PULLUP);
  pinMode(BUTTON_R, INPUT);
  #endif
#endif
}

