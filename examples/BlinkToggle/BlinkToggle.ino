/*
   Copyright (c) 2017, The STEAM Train

   Example Blink toggle program.

*/
#include <STEAMbot.h>

void setup() {
}

void loop() {
  sb.rgb.toggle();
  delay(1000);
}
