/*
   Copyright (c) 2017, The STEAM Train

   Example Blink program.

*/
#include <STEAMbot.h>

void setup() {
  sb.rgb.setOnColor(sbRGB::red);
}

void loop() {
  sb.rgb.on();
  delay(1000);
  sb.rgb.off();
  delay(500);
}
