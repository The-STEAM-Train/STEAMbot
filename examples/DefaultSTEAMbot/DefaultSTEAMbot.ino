/*
   Copyright (c) 2017, The STEAM Train

   Default program for a STEAMbot robot. Written by Aram Perez

*/

#include <STEAMbot.h>

//#define DEBUGGING_BT    //If defined, debugging information is sent to the Serial port (USB)
//#define DO_LINE_MODE    //If you have the Line Following Add-on Kit, uncomment this line

#define STOPPING_DISTANCE 15  //centimeters
#define FAR_FORWARD_DISTANCE 35
#define NEAR_FORWARD_DISTANCE 20
#define BACKWARD_DISTANCE 10
#define PET_FORWARD_SPEED 50

#define BUTTON_DEBOUNCE 50

#ifdef DO_LINE_MODE
#define LEFT_LINE_SENSOR_PIN PB5
#define RIGHT_LINE_SENSOR_PIN PB2

#define FORWARD_SPEED 35
#define TURN_SPEED (FORWARD_SPEED - 10)

sbDigitalInput lineSensorL(LEFT_LINE_SENSOR_PIN);
sbDigitalInput lineSensorR(RIGHT_LINE_SENSOR_PIN);
#endif

sbTimer rgbTimer(500, true);  //Auto restart timer

typedef enum movingState_t {
  movingBackward = -1,
  stopped,
  turning,
  movingForward
};
movingState_t movingState = stopped;

typedef enum modes {
  bluetooth,    //Controlled over Bluetooth
  pet,          //Pet mode, move forward/backward depending on USS
  line          //Follow a line
} modes_t;
mode_t currentMode;

bool useUss = false;

#ifdef DO_LINE_MODE
typedef enum lineModes {
  waitingForButton,
  following
} lineModes_t;
lineModes_t lineMode;
#endif

//const char csVersion[] = "STEAMbot v1.0.0";

unsigned melodyNotes[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

void showChange(int nbr, sbRGB::color_t newColor) {
  unsigned startNote = 300;
  for ( int i = 0; i < nbr; i++ ) {
    sb.speaker.playNote(startNote + i * 300, 333);
  }
  for (int i = 0; i < 8; i++ ) {
    sb.rgb.nextColor();
    delay(100);
  }
  sb.rgb.setColor(newColor);
#ifdef DEBUGGING_BT
  Serial.print("New mode: ");
  Serial.println(nbr);
#endif
}

void switchToBluetoothMode() {
  sb.motors.stop();
  currentMode = bluetooth;
  movingState = stopped;
  showChange(1, sbRGB::blue);
}

inline void switchToPetMode()
{
  sb.motors.stop();
  currentMode = pet;
  rgbTimer.start();
  showChange(2, sbRGB::fuchsia);
}

#ifdef DO_LINE_MODE
void switchToLineMode() {
  sb.motors.stop();
  currentMode = line;
  lineMode = waitingForButton;
  rgbTmr.start();
  showChange(3, sbRGB::yellow);
}
#endif

void setup()
{
#if defined(DEBUGGING_BT)
  Serial.begin(115200);
  delay(3000);
  Serial.println("Initializing STEAMbot");
#endif
  randomSeed(analogRead(0));
  sbBt.begin(SB_BT_BAUD);
  checkBtName();
  switchToBluetoothMode();
#if defined(DEBUGGING_BT)
  Serial.println("STEAMbot is ready.");
#endif
}

int getSignedByte() {
  uint8_t val;
  do {
    val = sbBt.read();
  } while ( val == 0xFF );
  if ( val & 0x80 ) { //If negative
    return val | 0xFFFFFF00;  //Sign extend
  }
  return val;
}

void doBluetoothMode()
{
  static uint8_t currentColor = 0;
  int val1, val2;
  uint8_t cmd;
  sbRGB::color_t color;
  float cm;

  if (sbBt.available() > 0) {
    cmd = sbBt.read();
    sbBt.write(cmd);
#ifdef DEBUGGING_BT
    Serial.write(cmd);
#endif
    switch (cmd) {
      case 1:   //Move command
        // set left and right motor speeds
        val1 = getSignedByte();
        val2 = getSignedByte();
        sb.motors.run(val1, val2);
        sbBt.write('\r');
        if ( val1 == 0 && val2 == 0 ) {
          movingState = stopped;
        } else if ( val1 > 0 && val2 > 0 ) {
          movingState = movingForward;
        } else if ( val1 < 0 && val2 < 0 ) {
          movingState = movingBackward;
        } else {
          movingState = turning;
        }
#ifdef DEBUGGING_BT
        Serial.print(val1);
        Serial.print(',');
        Serial.println(val2);
#endif
        break;
      case 'i':
        // inform about robot
        sbBt.print(STEAMbot::version);
#ifdef DEBUGGING_BT
        Serial.println(STEAMbot::version);
#endif
        break;
      case 'R':
        sb.motors.stop();
        sbBt.write('\r');
        movingState = stopped;
#ifdef DEBUGGING_BT
        Serial.println();
#endif
        break;
      case 'r':
        val1 = getSignedByte() & 7;
        sb.rgb.setColor(val1);
        sbBt.write('\r');
#ifdef DEBUGGING_BT
        Serial.println(val1);
#endif
        break;
      case 'b':   //Beep
        sb.speaker.beep();
        sbBt.write('\r');
#ifdef DEBUGGING_BT
        Serial.println();
#endif
        break;
      case 'n':
        val1 = sb.rgb.nextColor();
        sbBt.write(val1);
        sbBt.write('\r');
#ifdef DEBUGGING_BT
        Serial.println(val1);
#endif
        break;
      case '\r':
      case '\n':
        //ignore CR and NL
        break;
      case 'c':
        cm = sb.ultrasonic.centimeters();
        sbBt.print(cm);   //Send distance in ASCII
        sbBt.write('\r');
#ifdef DEBUGGING_BT
        Serial.println(cm);
#endif
        break;
      case 'm':
        for (int i = 0; i < sizeof(melodyNotes) / sizeof(melodyNotes[0]); i++ ) {
          if ( i == 1 || i == 2 ) {
            sb.speaker.playNote(melodyNotes[i], 125);
          } else {
            sb.speaker.playNote(melodyNotes[i], 250);
          }
        }
        break;
      case '`':
        sbBt.write('\r');
        while (true) {
          val1 = sbBt.read();
          if ( val1 == '\r' ) {
            break;
          }
#ifdef DEBUGGING_BT
          Serial.print(val1, HEX);
          Serial.print(' ');
#endif
        }
#ifdef DEBUGGING_BT
        Serial.println();
#endif
        break;
      case 'u':
        useUss = !useUss;
        sbBt.write('\r');
#ifdef DEBUGGING_BT
        Serial.println(cmd);
#endif
        break;
      default:
        // inform user of non existing command
        sbBt.print("?\r");
#ifdef DEBUGGING_BT
        Serial.println("?");
#endif
    }
    sbBt.flush();
  }
  if ( sb.runButton.isPressed(BUTTON_DEBOUNCE) ) {
    sb.speaker.beep();
  }
  if ( useUss && movingState == movingForward ) {
    cm = sb.ultrasonic.centimeters();
    if ( cm < STOPPING_DISTANCE ) {
      delay(100);
      cm = sb.ultrasonic.centimeters();
      if ( cm < STOPPING_DISTANCE ) {
        sb.motors.stop();
        movingState = stopped;
#ifdef DEBUGGING_BT
        Serial.print("-Stopped, obstacle at ");
        Serial.print(cm);
        Serial.println(" cm.");
#endif
      }
    }
  }
}

void doPetMode()
{
  float cm = sb.ultrasonic.centimeters();
  if ( cm < BACKWARD_DISTANCE ) {
    switch ( movingState ) {
      case movingForward:
        sb.motors.stop();
        delay(300);
      //Fall through
      case stopped:
        sb.motors.run(-random(20, 100), -random(20, 100));
        sb.rgb.setOnColor(sbRGB::red);
        movingState = movingBackward;
#ifdef DEBUGGING_BT
        Serial.print("b cm: ");
        Serial.println(cm);
#endif
        break;
    }
  } else if ( cm > NEAR_FORWARD_DISTANCE && cm < FAR_FORWARD_DISTANCE ) {
    switch ( movingState ) {
      case movingBackward:
        sb.motors.stop();
#ifdef DEBUGGING_BT
        Serial.print("f cm: ");
        Serial.println(cm);
#else
        delay(300);
#endif
      //Fall through
      case stopped:
        sb.motors.run(PET_FORWARD_SPEED, PET_FORWARD_SPEED);
        sb.rgb.setOnColor(sbRGB::fuchsia);
        movingState = movingForward;
        break;
    }
  } else {
    if ( movingState != stopped ) {
      sb.motors.stop();
      movingState = stopped;
      sb.rgb.setOnColor(sbRGB::aqua);
#ifdef DEBUGGING_BT
      Serial.print("s cm: ");
      Serial.println(cm);
#endif
    }
  }
  if ( rgbTimer.timedOut() ) {
    sb.rgb.toggle();
  }
  delay(100);
}

#ifdef DO_LINE_MODE
void doLineMode() {
  static bool finished;
  bool wait;
  int movement;
  switch ( lineMode ) {
    case waitingForButton:
      if ( sb.runButton.isPressed(BUTTON_DEBOUNCE) ) {
        lineMode = following;
        finished = false;
        sb.rgb.setColor(sbRGB::green);
        sb.motors.run(FORWARD_SPEED, FORWARD_SPEED);
        sb.speaker.beep();
      }
      break;
    default:
      wait = false;
      movement = 0;
      movement |= lineSensorL.read() ? 1 : 0;
      movement |= lineSensorR.read() ? 2 : 0;
      switch ( movement ) {
        case 1:
          sb.motors.run(TURN_SPEED, -TURN_SPEED);  //Turn right
          wait = true;
          //          Serial.print('R');
          break;
        case 2:
          sb.motors.run(-TURN_SPEED, TURN_SPEED);  //Turn left
          wait = true;
          //          Serial.print('L');
          break;
        case 3:
          sb.motors.run(FORWARD_SPEED, FORWARD_SPEED);
          //          Serial.print('F');
          break;
        default:
          //          Serial.print('S');
          finished = true;
      }
      if ( wait ) {
        do {
          bool overLineL = lineSensorL.read();
          bool overLineR = lineSensorR.read();
          if ( overLineL && overLineR ) break;
          if ( !overLineL && !overLineR ) {
            finished = true;
            break;
          }
        } while ( true );
      }
      if ( finished ) {
        sb.motors.stop();
        sb.speaker.beep();
        delay(100);
        sb.speaker.beep();
        lineMode = waitingForButton;
      }
  }
  if ( rgbTmr.timedOut() ) {
    sb.rgb.toggle();
  }
}
#endif

void loop() {
  if ( sb.stopButton.isPressed(BUTTON_DEBOUNCE) ) {
#ifdef DEBUGGING_BT
    Serial.print("Switching modes... ");
#endif
    switch ( currentMode ) {
      case bluetooth:
        switchToPetMode();
        break;
#ifdef DO_LINE_MODE
      case pet:
        switchToLineMode();
        break;
#endif
      default:
        switchToBluetoothMode();
    }
  }
  switch ( currentMode ) {
    case pet:
      doPetMode();
      break;
#ifdef DO_LINE_MODE
    case line:
      doLineMode();
      break;
#endif
    default:
      doBluetoothMode();
  }
}

void checkBtName() {
  char bfr[65];
  byte nbrRead;

#if defined(DEBUGGING_BT)
  Serial.println("Setting connection interval to 10ms");
#endif
  delay(150);
  sbBt.print("AT+NEIN0");
  nbrRead = sbBt.readBytesUntil(10, bfr, sizeof(bfr) - 1);
  if ( nbrRead == 0 ) {
#if defined(DEBUGGING_BT)
    Serial.println("error.");
#endif
    return;
  }
#if defined(DEBUGGING_BT)
  Serial.print("Getting current name: ");
  memset(bfr, 0, sizeof(bfr));
#endif
  delay(200);
  sbBt.print("AT+NAME");
  nbrRead = sbBt.readBytesUntil(10, bfr, sizeof(bfr) - 1);
  if ( nbrRead == 0 ) {
#if defined(DEBUGGING_BT)
    Serial.println("error.");
#endif
    return;
  }
#if defined(DEBUGGING_BT)
  Serial.println(&bfr[6]);
#endif
  if ( strncmp(&bfr[6], "STEAMbot-", 9) == 0 ) {
    return;
  }
  delay(200);
#if defined(DEBUGGING_BT)
  Serial.println("Getting MAC addr: ");
#endif
  sbBt.print("AT+MAC");
  nbrRead = sbBt.readBytesUntil(10, bfr, sizeof(bfr) - 1);
  if ( nbrRead == 0 ) {
#if defined(DEBUGGING_BT)
    Serial.println("error.");
#endif
    return;
  }
#if defined(DEBUGGING_BT)
  Serial.println(&bfr[5]);
#endif
  String newName("STEAMbot-");
  newName += bfr + 11;
  String newNameCmd("AT+NAME");
  newNameCmd += newName;
  delay(200);
#if defined(DEBUGGING_BT)
  Serial.print("Setting new name to: ");
  Serial.println(newName);
#endif
  sbBt.print(newNameCmd);
  delay(500);
  nbrRead = sbBt.readBytesUntil(10, bfr, sizeof(bfr) - 1);
  if ( nbrRead == 0 ) {
#if defined(DEBUGGING_BT)
    Serial.println("error.");
#endif
    return;
  }
#if defined(DEBUGGING_BT)
  Serial.println("Name changed successfully!");
#endif
}

