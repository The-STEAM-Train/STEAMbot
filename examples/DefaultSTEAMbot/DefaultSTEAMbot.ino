/*
   Copyright (c) 2017, 2018, The STEAM Train

   Default program for a STEAMbot robot. Written by Aram Perez

*/

#include <STEAMbot.h>
#include <Servo.h>

#define DEBUGGING_BT    //If defined, debugging information is sent to the Serial port (USB)
//#define DO_LINE_MODE    //If you have the Line Following Add-on Kit, uncomment this line

#define CR '\r'
#define LF '\n'

#define STOPPING_DISTANCE 15 //centimeters
#define FAR_FORWARD_DISTANCE 35
#define NEAR_FORWARD_DISTANCE 20
#define BACKWARD_DISTANCE 10
#define PET_FORWARD_SPEED 50

#define BUTTON_DEBOUNCE 50
#define NOTE_DURATION 80

#ifdef DO_LINE_MODE
#define LEFT_LINE_SENSOR_PIN PB5
#define RIGHT_LINE_SENSOR_PIN PB2

#define FORWARD_SPEED 35
#define TURN_SPEED (FORWARD_SPEED - 10)

sbDigitalInput lineSensorL(LEFT_LINE_SENSOR_PIN);
sbDigitalInput lineSensorR(RIGHT_LINE_SENSOR_PIN);
#endif

sbTimer rgbTimer(500, true); //Auto restart timer
Servo srv1, srv2, srv3;
bool s1Configured, s2Configured, s3Configured;

const char defaultProgramVersion[] = "1.1.1";

typedef enum movingState_t {
  movingBackward = -1,
  stopped,
  turning,
  movingForward
};
movingState_t movingState = stopped;

typedef enum modes {
  bluetooth, //Bluetooth controlled mode
  pet,       //Pet mode, move forward/backward depending on USS
  line       //Line following mode
} modes_t;
mode_t currentMode;

bool autoStop = false;

#ifdef DO_LINE_MODE
typedef enum lineModes {
  waitingForButton,
  following
} lineModes_t;
lineModes_t lineMode;
#endif

unsigned melodyNotes[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

void showChange(int nbr, sbRGB::color_t newColor)
{
  unsigned startNote = 300;
  for (int i = 0; i < nbr; i++)
  {
    sb.speaker.playNote(startNote + i * 300, 333);
  }
  for (int i = 0; i < 8; i++)
  {
    sb.rgb.nextColor();
    delay(100);
  }
  sb.rgb.setColor(newColor);
#ifdef DEBUGGING_BT
  Serial.print("New mode: ");
  Serial.println(nbr);
#endif
}

void switchToBluetoothMode()
{
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
void switchToLineMode()
{
  sb.motors.stop();
  currentMode = line;
  lineMode = waitingForButton;
  rgbTimer.start();
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
  s1Configured = s2Configured = s3Configured = false;
#if defined(DEBUGGING_BT)
  Serial.println("STEAMbot is ready.");
#endif
}

uint8_t getByte()
{
  while ( sbBt.available() == 0 );
  return (uint8_t) sbBt.read();
}

uint16_t getUint16() //Big endian
{
  uint16_t val = getByte() << 8;
  val |= getByte();
  return val;
}

bool getPin(uint8_t& pinNbr) {
  static const uint8_t pinMap[] = {PA8, PB2, PB5, PB6, PB7};
  bool rv = false;
  uint8_t ndx = getByte();
  if ( ndx < sizeof(pinMap) ) {
    pinNbr = pinMap[ndx];
    rv = true;
  }
  return rv;
}

void doBluetoothMode()
{
  static uint8_t currentColor = 0;
  int8_t iv1, iv2;
  uint8_t cmd, pin, uiv;
  uint16_t freq, dur;
  sbRGB::color_t color;
  float cm;

  if (sbBt.available() > 0)//Big endian
  {
    cmd = sbBt.read();
    switch (cmd)
    {
      case 1: //Move left and right motors
        iv1 = (int8_t) getByte();
        iv2 = (int8_t) getByte();
        sb.motors.run(iv1, iv2);
        if (iv1 == 0 && iv2 == 0)
        {
          movingState = stopped;
        }
        else if (iv1 > 0 && iv2 > 0)
        {
          movingState = movingForward;
        }
        else if (iv1 < 0 && iv2 < 0)
        {
          movingState = movingBackward;
        }
        else
        {
          movingState = turning;
        }
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.print(cmd);
        Serial.print(iv1);
        Serial.print(',');
        Serial.print(iv2);
#endif
        break;
      case 'B': //Return RUN or STOP button state
        pin = getByte();
        switch ( pin ) {
          case 0:
            uiv = sb.runButton.isPressed(BUTTON_DEBOUNCE) ? 1 : 0;
            break;
          case 1:
            uiv = sb.stopButton.isPressed(BUTTON_DEBOUNCE) ? 1 : 0;
            break;
        }
        sbBt.write(cmd);
        sbBt.write(uiv);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.print(cmd);
        Serial.print(pin);
        Serial.print(',');
        Serial.print(uiv);
#endif
        break;
      case 'M': //Set pin mode (PA8, PB2, PB5, PB6, PB7)
        pin = uiv = 0xFF;
        if ( getPin(pin) ) {
          uiv = getByte();
          switch ( uiv ) {
            case 0:
              pinMode(pin, INPUT);
              break;
            case 1:
              pinMode(pin, INPUT_PULLUP);
              break;
            case 2:
              pinMode(pin, OUTPUT);
              break;
          }
        }
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(pin);
        Serial.print(',');
        Serial.print(uiv);
#endif
        break;
      case 'o': //Output to a pin (PA8, PB2, PB5, PB6, PB7)
        pin = uiv = 0xFF;
        if ( getPin(pin) ) {
          uiv = getByte();
          digitalWrite(pin, uiv != 0 ? HIGH : LOW );
        }
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(pin);
        Serial.print(',');
        Serial.print(uiv);
#endif
        break;
      case 'i': //Return the input level of a pin (PA8, PB2, PB5, PB6, PB7)
        pin = 0xFF;
        uiv = 0;
        if ( getPin(pin) ) {
          uiv = digitalRead(pin);
        }
        sbBt.write(cmd);
        sbBt.write(uiv);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(pin);
        Serial.print(',');
        Serial.print(uiv);
#endif
        break;
      case 'v': // Get version numbers, first the library versio and then the this program's version
        sbBt.write(cmd);
        sbBt.print(STEAMbot::version);
        sbBt.write(',');
        sbBt.print(defaultProgramVersion);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(STEAMbot::version);
        Serial.write(',');
        Serial.print(defaultProgramVersion);
#endif
        break;
      case 'r': //Set the color of the RGB LED
        uiv = getByte() & 7;
        sb.rgb.setColor(uiv);
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(uiv);
#endif
        break;
      case 'n': //Set the next color of the RGB LED
        uiv = sb.rgb.nextColor();
        sbBt.write(cmd);
        sbBt.write(uiv);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(uiv);
#endif
        break;
      case 'b': //Beep
        sb.speaker.beep();
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
#endif
        break;
      case 'm': //Play the builtin melody
        for (int i = 0; i < sizeof(melodyNotes) / sizeof(melodyNotes[0]); i++)
        {
          if (i == 1 || i == 2)
          {
            sb.speaker.playNote(melodyNotes[i], NOTE_DURATION);
          }
          else
          {
            sb.speaker.playNote(melodyNotes[i], 2 * NOTE_DURATION);
          }
        }
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
#endif
        break;
      case 'c': //Get the distance in centimeters
        cm = sb.ultrasonic.centimeters();
        cm += sb.ultrasonic.centimeters();
        cm /= 2;
        sbBt.write(cmd);
        sbBt.print(cm); //Send distance in ASCII
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(cm);
#endif
        break;
      case 't': //Play a note (tone)
        freq = getUint16();
        dur = getUint16();
        sb.speaker.playNote(freq, dur);
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(freq);
        Serial.print(',');
        Serial.print(dur);
#endif
        break;
      case 'u': //Toggle autostop
        autoStop = !autoStop;
        sbBt.write(cmd);
        sbBt.write(autoStop ? 1 : 0);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(autoStop);
#endif
        break;
      case 's': //Set angle of a servo (SRV1/PB7, SRV2/PB6)
        pin = getByte();
        uiv = getByte(); //Get the angle
        if ( uiv > 180 ) {
          uiv = 180;
        }
        switch ( pin ) {
          case 0:
            if ( s1Configured ) {
              srv1.write(uiv);
            }
            break;
          case 1:
            if ( s2Configured ) {
              srv2.write(uiv);
            }
            break;
          case 3:
            if ( s3Configured ) {
              srv3.write(uiv);
            }
            break;
        }
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(pin);
        Serial.print(',');
        Serial.print(uiv);
#endif
        break;
      case 'S': //Configure a servo
        pin = getByte();
        switch ( pin ) {
          case 0:
            if ( !s1Configured ) {
              srv1.attach(PB7);
              s1Configured = true;
            }
            break;
          case 1:
            if ( !s2Configured ) {
              srv2.attach(PB6);
              s2Configured = true;
            }
            break;
          case 2:
            if ( !s3Configured ) {
              srv3.attach(PA8);
              s3Configured = true;
            }
            break;
        }
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.write(cmd);
        Serial.print(pin);
#endif
        break;
      case CR: //ignore CR and LF
      case LF:
        break;
      default:
        // inform user of non existing command
        sbBt.write('?');
        sbBt.write(cmd);
#ifdef DEBUGGING_BT
        sbBt.write(CR);
        sbBt.flush();
        Serial.print('?');
        Serial.print(cmd, HEX);
#endif
    }
#ifdef DEBUGGING_BT
    Serial.println();
#else
    sbBt.write(CR);
    sbBt.flush();
#endif
  }
  if (autoStop && movingState == movingForward)
  {
    cm = sb.ultrasonic.centimeters();
    if (cm < STOPPING_DISTANCE)
    {
      delay(50);
      cm = sb.ultrasonic.centimeters();
      if (cm < STOPPING_DISTANCE)
      {
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
  if (cm < BACKWARD_DISTANCE)
  {
    switch (movingState)
    {
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
  }
  else if (cm > NEAR_FORWARD_DISTANCE && cm < FAR_FORWARD_DISTANCE)
  {
    switch (movingState)
    {
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
  }
  else
  {
    if (movingState != stopped)
    {
      sb.motors.stop();
      movingState = stopped;
      sb.rgb.setOnColor(sbRGB::aqua);
#ifdef DEBUGGING_BT
      Serial.print("s cm: ");
      Serial.println(cm);
#endif
    }
  }
  if (rgbTimer.timedOut())
  {
    sb.rgb.toggle();
  }
  delay(100);
}

#ifdef DO_LINE_MODE
void doLineMode()
{
  static bool finished;
  bool wait;
  int movement;
  switch (lineMode)
  {
    case waitingForButton:
      if (sb.runButton.isPressed(BUTTON_DEBOUNCE))
      {
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
      switch (movement)
      {
        case 1:
          sb.motors.run(TURN_SPEED, -TURN_SPEED); //Turn right
          wait = true;
          break;
        case 2:
          sb.motors.run(-TURN_SPEED, TURN_SPEED); //Turn left
          wait = true;
          break;
        case 3:
          sb.motors.run(FORWARD_SPEED, FORWARD_SPEED);
          break;
        default:
          finished = true;
      }
      if (wait)
      {
        do
        {
          bool overLineL = lineSensorL.read();
          bool overLineR = lineSensorR.read();
          if (overLineL && overLineR)
            break;
          if (!overLineL && !overLineR)
          {
            finished = true;
            break;
          }
        } while (true);
      }
      if (finished)
      {
        sb.motors.stop();
        sb.speaker.beep();
        delay(100);
        sb.speaker.beep();
        lineMode = waitingForButton;
      }
  }
  if (rgbTimer.timedOut())
  {
    sb.rgb.toggle();
  }
}
#endif

void loop()
{
  if (sb.runButton.isPressed(BUTTON_DEBOUNCE) && sb.stopButton.isPressed(BUTTON_DEBOUNCE))
  {
#ifdef DEBUGGING_BT
    Serial.print("Switching modes... ");
#endif
    switch (currentMode)
    {
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
  switch (currentMode)
  {
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

void checkBtName()
{
  char bfr[65];
  byte nbrRead;

#if defined(DEBUGGING_BT)
  Serial.println("Setting connection interval to 10ms");
#endif
  delay(150);
  sbBt.print("AT+NEIN0");
  nbrRead = sbBt.readBytesUntil(10, bfr, sizeof(bfr) - 1);
  if (nbrRead == 0)
  {
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
  if (nbrRead == 0)
  {
#if defined(DEBUGGING_BT)
    Serial.println("error.");
#endif
    return;
  }
#if defined(DEBUGGING_BT)
  Serial.println(&bfr[6]);
#endif
  if (strncmp(&bfr[6], "STEAMbot-", 9) == 0)
  {
    return;
  }
  delay(200);
#if defined(DEBUGGING_BT)
  Serial.println("Getting MAC addr: ");
#endif
  sbBt.print("AT+MAC");
  nbrRead = sbBt.readBytesUntil(10, bfr, sizeof(bfr) - 1);
  if (nbrRead == 0)
  {
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
  if (nbrRead == 0)
  {
#if defined(DEBUGGING_BT)
    Serial.println("error.");
#endif
    return;
  }
#if defined(DEBUGGING_BT)
  Serial.println("Name changed successfully!");
#endif
}

