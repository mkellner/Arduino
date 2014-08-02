// (c) 2014 mkellner@robotranch.org

#include "Arduino.h"
#include "DirSwitch.h"

char switchPos = 0;
char dir = 0;

char checkSwitch() {
  char changed = 0;
  if (digitalRead(SW_DOWN)) {
    Serial.print("Dn ");
    if (switchPos != -1) {
      switchPos = -1;
      changed = 1;
    }
  }
  else if (digitalRead(SW_UP)) {
    Serial.print("Up ");
    if (switchPos != 1) {
      switchPos = 1;
      changed = 1;
    }
  }
  else {
    if (switchPos != 0) {
      switchPos = 0;
      changed = 1;
    }
  }
  return changed;
}


