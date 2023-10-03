#ifdef TDISPLAY
#include <Arduino.h>

bool buttonRight(){
  return (digitalRead(BUTTON_R) == 0);
}

bool buttonLeft(){
  return (digitalRead(BUTTON_L) == 0);
}

bool buttonPowerOff(){
  return (digitalRead(BUTTON_L) == 0 && digitalRead(BUTTON_R) == 0);
}

void buttonInit(){
  pinMode(BUTTON_L, INPUT_PULLUP);
  pinMode(BUTTON_R, INPUT);
}
#endif

