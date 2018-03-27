#ifndef STEAMbot_H
#define STEAMbot_H
/*
  The STEAMbot Library

  Written by Aram Perez

  The MIT License (MIT)
  Copyright © 2017 The STEAM Train

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation   files (the “Software”), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
#include <Arduino.h>
#include <limits.h>

#if !defined(ARDUINO_MAPLE_MINI) //Based on Leaf Maple Mini Library
#pragma err This library requires a STEAMbot Controller (Maple Mini)
#endif

#define MAX_PWM_VALUE 0xFFFF
#define MAX_MOTOR_SPEED MAX_PWM_VALUE
#define MIN_MOTOR_SPEED 0

#define LED_OFF HIGH
#define LED_ON LOW

// GPIO Connections
#define SPEAKER_PIN PA6
#define RED_LED_PIN PB0
#define BLUE_LED_PIN PA7
#define GREEN_LED_PIN PB1
#define L1A_PIN PA0          //Left motor 1A
#define L2A_PIN PA4          //Left motor 1B
#define R1A_PIN PA1          //Right motor 1A
#define R2A_PIN PA5          //Right motor 1B
#define USS_TRIG_PIN PA14    //Ultrasonic sensor trigger on connector P5
#define USS_ECHO_PIN PA13    //Ultrasonic sensor echo on connector P5
#define RUN_BUTTON_PIN PB8   //Run button
#define STOP_BUTTON_PIN PC14 //Stop button

#define sbBt Serial2

#define SB_BT_BAUD 115200 //Bluetooth baud rate
#define SB_BT_RESET PC15

//
//Note values, taken from the Arduino Examples->02.Digital->toneMelody example code
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

#define BEEP_FREQ NOTE_A5

class sbDigitalInput
{
  uint8_t _pin;

  sbDigitalInput();

public:
  sbDigitalInput(uint8_t pin, bool pullup = false) : _pin(pin)
  {
    pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
  }
  virtual ~sbDigitalInput() {}
  inline bool read() const
  {
    return digitalRead(_pin);
  }
};

class sbDigitalOutputBase
{
  sbDigitalOutputBase();

protected:
  uint8_t _pin;

public:
  sbDigitalOutputBase(uint8_t pin) : _pin(pin)
  {
    pinMode(pin, OUTPUT);
  }
  virtual ~sbDigitalOutputBase() {}
  virtual void write(bool value) const = 0;
  virtual inline void on() const
  {
    this->write(true);
  }
  virtual inline void off() const
  {
    this->write(false);
  }
  virtual bool state() const = 0;
  inline void toggle() const
  {
    digitalWrite(_pin, digitalRead(_pin) ? false : true);
  }
};

class sbDigitalOutput : public sbDigitalOutputBase
{
  sbDigitalOutput();

public:
  sbDigitalOutput(uint8_t pin, bool initVal = false) : sbDigitalOutputBase(pin)
  {
    this->write(initVal);
  }
  virtual ~sbDigitalOutput() {}
  inline void write(bool value) const
  {
    digitalWrite(_pin, value);
  }
  inline bool state() const
  {
    return digitalRead(_pin);
  }
};

class sbDigitalOutputAL : public sbDigitalOutputBase
{
  sbDigitalOutputAL();

public:
  sbDigitalOutputAL(uint8_t pin, bool initVal = false) : sbDigitalOutputBase(pin)
  {
    this->write(initVal);
  }
  virtual ~sbDigitalOutputAL() {}
  inline void write(bool value) const
  {
    digitalWrite(_pin, value ? false : true);
  }
  inline bool state() const
  {
    return !digitalRead(_pin);
  }
};

class sbButton : sbDigitalInput
{
  sbButton();

public:
  sbButton(uint8_t pin) : sbDigitalInput(pin)
  {
  }
  virtual ~sbButton() {}
  bool isPressed(unsigned ms = 0) const
  {
    bool pressed = this->read();
    if ((ms > 0) && pressed)
    {
      delayMicroseconds(ms);
      if (pressed != this->read())
      {
        return false;
      }
    }
    return pressed;
  }
};

class sbRGB
{
public:
  typedef enum colors {
    black,
    blue,
    green,
    aqua,
    red,
    fuchsia,
    yellow,
    white
  } color_t;

private:
  sbDigitalOutputAL _red;
  sbDigitalOutputAL _green;
  sbDigitalOutputAL _blue;
  color_t _currentColor;
  color_t _onColor;
  color_t _offColor;

  sbRGB();

public:
  sbRGB(uint8_t redPin, uint8_t greenPin, uint8_t bluePin) : _red(redPin), _green(greenPin), _blue(bluePin)
  {
    _onColor = sbRGB::white;
    _offColor = sbRGB::black;
  }
  virtual ~sbRGB()
  {
    off();
  }
  void setColor(color_t color)
  {
    _currentColor = color;
    _currentColor & 4 ? _red.on() : _red.off();
    _currentColor & 2 ? _green.on() : _green.off();
    _currentColor & 1 ? _blue.on() : _blue.off();
  }
  inline void setColor(int color)
  {
    setColor((color_t)(color & 7));
  }
  inline color_t color() const
  {
    return _currentColor;
  }
  color_t nextColor()
  {
    setColor((_currentColor + 1) & 7);
    return _currentColor;
  }
  inline void setOnColor(color_t color)
  {
    _onColor = color;
  }
  inline color_t onColor() const
  {
    return _onColor;
  }
  inline void on()
  {
    setColor(_onColor);
  }
  inline void setOffColor(color_t color)
  {
    _offColor = color;
  }
  inline color_t offColor() const
  {
    return _offColor;
  }
  inline void off()
  {
    setColor(_offColor);
  }
  inline void toggle()
  {
    _currentColor == _offColor ? on() : off();
  }
};

class sbSpeaker
{
  uint8_t _pin;

  sbSpeaker();

public:
  sbSpeaker(uint8_t pin) : _pin(pin)
  {
  }
  virtual ~sbSpeaker()
  {
    noTone();
  }
  void playNote(unsigned freq, unsigned dur, unsigned long restDur = 333) const
  {
    ::tone(_pin, freq, dur);
    delay(restDur);
  }
  inline void tone(unsigned freq) const
  {
    ::tone(_pin, freq);
  }
  inline void tone(unsigned freq, unsigned long dur) const
  {
    ::tone(_pin, freq, dur);
  }
  inline void noTone() const
  {
    ::noTone(_pin);
  }
  void beep(unsigned long dur = 100) const
  {
    ::tone(_pin, BEEP_FREQ, dur);
  }
};

class sbMotor
{
  uint8_t _ia;
  uint8_t _ib;
  typedef enum { movingBackward,
                 stopped,
                 movingForward } movement_t;
  movement_t lastMovement;

  sbMotor(); //private default constructor

public:
  sbMotor(uint8_t ia, uint8_t ib) : _ia(ia), _ib(ib)
  {
    pinMode(ia, PWM);
    pinMode(ib, OUTPUT);
    stop();
  }
  virtual ~sbMotor()
  {
    stop();
  }
  void stop()
  {
    pwmWrite(_ia, 0);
    digitalWrite(_ib, LOW);
    lastMovement = stopped;
  }
  void run(int16_t speed)
  {
    if (speed == 0)
    {
      stop();
    }
    else
    {
      if (speed > 0)
      {
        if (lastMovement == movingBackward)
        {
          stop();
          delay(100);
        }
        lastMovement = movingForward;
        speed = MAX_MOTOR_SPEED - (speed * (MAX_MOTOR_SPEED - MIN_MOTOR_SPEED) / 100 + MIN_MOTOR_SPEED);
        digitalWrite(_ib, HIGH);
      }
      else
      {
        if (lastMovement == movingForward)
        {
          stop();
          delay(100);
        }
        lastMovement = movingBackward;
        speed = -speed * (MAX_MOTOR_SPEED - MIN_MOTOR_SPEED) / 100 + MIN_MOTOR_SPEED;
        digitalWrite(_ib, LOW);
      }
      pwmWrite(_ia, speed);
    }
  }
};

class sbDualMotors
{
  sbMotor leftMotor;
  sbMotor rightMotor;

  sbDualMotors();

public:
  sbDualMotors(uint8_t lia, uint8_t lib, uint8_t ria, uint8_t rib) : leftMotor(lia, lib), rightMotor(ria, rib)
  {
  }
  virtual ~sbDualMotors()
  {
    stop();
  }
  void stop()
  {
    leftMotor.stop();
    rightMotor.stop();
  }
  void run(int16_t leftSpeed, int16_t rightSpeed)
  {
    leftMotor.run(leftSpeed);
    rightMotor.run(-rightSpeed);
  }
};

class sbUSS
{
  uint8_t _trigPin;
  uint8_t _echoPin;
  unsigned long _delay;

  sbUSS();
  unsigned long pulseWidth() const
  {
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    unsigned long ms = pulseIn(_echoPin, HIGH, _delay);
    if (ms == 0)
    {
      return (unsigned long)(ULONG_MAX);
    }
    return ms;
  }

public:
  sbUSS(uint8_t trigPin, uint8_t echoPin, unsigned long defaultDelay = 5000) : _trigPin(trigPin),
                                                                               _echoPin(echoPin),
                                                                               _delay(defaultDelay)
  {
    pinMode(trigPin, OUTPUT);
    digitalWrite(trigPin, LOW);
    pinMode(echoPin, INPUT);
  }
  virtual ~sbUSS(){};
  inline void setDelay(unsigned long newDelay)
  {
    _delay = newDelay;
  }
  inline float inches() const
  {
    return pulseWidth() / 148.0; //Per data sheet
  }
  inline float centimeters() const
  {
    return pulseWidth() / 58.0; //Per data sheet
  }
};

class sbTimer
{
  unsigned long startTime;
  unsigned long duration;
  bool active;
  bool autoRestart;
  sbTimer();

public:
  sbTimer(unsigned long dur, bool autoRes = false) : duration(dur), active(false), autoRestart(autoRes) {}
  virtual ~sbTimer() {}
  void start(unsigned long dur = 0)
  {
    if (dur != 0)
    {
      duration = dur;
    }
    active = true;
    startTime = millis();
  }
  inline void stop() { active = false; }
  inline bool isActive() const { return active; }
  bool timedOut()
  {
    bool rv;
    if (active)
    {
      rv = ((millis() - startTime) > duration) ? true : false;
      if (rv)
      {
        if (autoRestart)
        {
          startTime = millis();
        }
        else
        {
          active = false;
        }
      }
    }
    else
    {
      rv = false;
    }
    return rv;
  }
  inline unsigned long remainingMs() const
  {
    return startTime + duration - millis();
  }
};

class STEAMbot
{
  STEAMbot();

public:
  static const char version[];
  sbButton runButton;
  sbButton stopButton;
  sbRGB rgb;
  sbSpeaker speaker;
  sbDualMotors motors;
  sbUSS ultrasonic;

  STEAMbot(uint8_t rBtnPin, uint8_t sBtnPin, uint8_t rPin, uint8_t gPin, uint8_t blPin,
           uint8_t spPin, uint8_t l1Pin, uint8_t l2Pin, uint8_t r1Pin, uint8_t r2Pin,
           uint8_t tPin, uint8_t ePin) : runButton(rBtnPin),
                                         stopButton(sBtnPin),
                                         rgb(rPin, gPin, blPin),
                                         speaker(spPin),
                                         motors(l1Pin, l2Pin, r1Pin, r2Pin),
                                         ultrasonic(tPin, ePin)
  {
    pinMode(SB_BT_RESET, OUTPUT);
    digitalWrite(SB_BT_RESET, HIGH);
    delay(10);
    digitalWrite(SB_BT_RESET, LOW);
    delay(10);
    digitalWrite(SB_BT_RESET, HIGH);
  }
  virtual ~STEAMbot() {}
};

const char STEAMbot::version[] = "1.0.2";

extern STEAMbot sb(RUN_BUTTON_PIN, STOP_BUTTON_PIN, RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN,
                   SPEAKER_PIN, L1A_PIN, L2A_PIN, R1A_PIN, R2A_PIN,
                   USS_TRIG_PIN, USS_ECHO_PIN);

#endif //STEAMbot_H
