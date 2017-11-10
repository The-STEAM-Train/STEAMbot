/*
   Copyright (c) 2017, The STEAM Train

   Example Alarm program.

*/
#include <STEAMbot.h>

void setup() {
  Serial.begin(115200);
  sb.rgb.setOnColor(sbRGB::red);
}

void loop() {
  float cm = sb.ultrasonic.centimeters();
  if ( cm < 10.0 ) {
    sb.rgb.on();
    sb.speaker.beep();
    Serial.println(cm);
  } else {
    sb.rgb.off();
  }
}
