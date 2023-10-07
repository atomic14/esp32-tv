#include <Arduino.h>
#ifdef M5CORE2
#include "M5Touch.h"
M5Touch Touch;
#endif

bool buttonRight(){
#ifdef M5CORE2
  TouchPoint_t pos   = Touch.getPressPoint();
  if(pos.x>100) return true;
#endif
#ifdef BUTTON_R
  return (digitalRead(BUTTON_R) == 0);
#endif
  return false;
}

bool buttonLeft(){
#ifdef M5CORE2
  TouchPoint_t pos   = Touch.getPressPoint();
  if(pos.x > 0 && pos.x<100) return true;
#endif
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
  Touch.begin();
#endif
}

