/*
   Copyright (c) 2017, The STEAM Train

   Example Pushbuttons program.

*/
#include <STEAMbot.h>

void setup() {
}

void loop() {
  if (sb.runButton.isPressed() && sb.stopButton.isPressed() ) {
    sb.rgb.off();
    delay(250);
  } else if ( sb.runButton.isPressed() ) {
    sb.rgb.setColor(sbRGB::green);
  } else if (sb.stopButton.isPressed() ) {
    sb.rgb.setColor(sbRGB::red);
  }
}
