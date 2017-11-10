# STEAMbot
The library for programming a STEAMbot robot by The STEAM Train.

# Description
This library contains an overall class for the STEAMbot components and several individual classes for the different inputs and outputs of the STEAMbot robot. The following classes are defined:

* _sbDigitalInput_ - class for general purpose digital inputs
* _sbDigitalOutput_ - class for general purpose digital outputs (active high)
* _sbDigitalOutputAL_ - class for general purpose digital outputs (active low)
* _sbButton_ - class that provides access to the onboard buttons (a sub-class of sbDigitalInput)
* _sbRGB_ - class to control the onboard RBG LED
* _sbSpeaker_ - class to produce sounds on the onboard speaker
* _sbMotor_ - class for controlling a single motor
* _sbDualMotors_ - class for controlling both motors of the STEAMbot robot (based on sbMotor)
* _sbUSS_ - class for measuring distance with a ultrasonic sensor
* _sbTimer_ - general purpose timer class
* _STEAMbot_ - is the major class with predefined members of the above classes; there is a single instance called **sb**.

The STEAMbox classes are described below.

* * *

# sbDigitalInput
Class for handling a digital input.
## sbDigitalInput(_pin_, _pullup_)
Creates a digital input object (constructor).
### Arguments
* uint8_t _pin_: the pin number of the LED.
* bool _pullup_: true = enable the pullup resistor, false = disable the pullup resistor. The default is _false_.

## bool read()
Returns the current state of the digital input (true = HIGH, false = LOW).

* * *

# sbButton
Class for checking a pushbutton.
## sbButton(_pin_)
### Arguments
* uint8_t _pin_: the pin number of the button.

## bool isPressed(_ms_)
Returns true if the button is pressed or false if the button is not pressed. If the optional _ms_ argument is zero (or no argument is provided), no debouncing is performed. However, if _ms_ is not zero, the button is debounced by delaying _ms_ microseconds and checking again.
### Arguments
* unsigned long _ms_: debounce duration in microseconds. If _ms_ is 0 (the default), no debouncing is performed.

* * *

# sbDigitalOutput and sbDigitalOutputAL
Class for handling a digital output, regular active high or an active low output.
## sbDigitalOutput(_pin_, bool initVal) or sbDigitalOutputAL(_pin_, bool initVal)
Creates a digital input object (constructor).
### Arguments
* uint8_t _pin_: the pin number of the LED.
* bool _initVal_: the initial value for the ouput.

## void write(_value_)
Writes value to the output pin. For active high, true = HIGH and false = LOW. For active low, true = LOW and false = HIGH.
### Arguments
* uint8_t _pin_: the pin number of the LED.

## void on()
This method is equivalent to write(true).
## void off()
This method is equivalent to write(false).
## bool state()
Returns the current state of the digital output. For active high, true = HIGH and false = LOW. For active low, true = LOW and false = HIGH.

## bool toggle()
If the state is true, set the output to false, otherwise set the output to true.

* * *

# sbRGB
Class for controlling an RGB LED. The following colors are supported:

* black (0)
* blue (1)
* green (2)
* aqua (3)
* red (4)
* fuchsia (5)
* yellow (6)
* white (7)

The colors are defined as an enum typedef'd as _sbRGB::color\_t_ with the values in parenthesis above.

## sbRBG(_redPin_, _greenPin_, _bluePin_)
Creates an RGB LED object (constructor). The RBG LED on the STEAMbot is a common anode LED. The _onColor_ is set to white and the _offColor_ is set to black.
### Arguments
* uint8_t _redPin_: the pin number of the red LED.
* uint8_t _greenPin_: the pin number of the green LED.
* uint8_t _bluePin_: the pin number of the blue LED.

## void setColor(_newColor_)
Sets a new color for the RGB LED.
### Arguments
* int _newColor_: New color for the RGB LED, range is 0 - 7 (numbers > 7 are mapped to the range of 0 - 7).

## color_t color()
Returns the current color of the RGB LED as an enum.

##void nextColor()
Changes to the next color in the list of supported colors.

##void setColor(_newColor_)
Sets a new color for the RGB LED.
### Arguments
*  color_t _newColor_: New color for the RGB LED. Values are _black_,  _blue_, _green_, _aqua_, _red_, _fushia_, _yellow_ or _white_.

##void setOnColor(_newColor_)
Sets the color of the _on_ state.
### Arguments
*  color_t _newColor_: New color for the RGB LED.

## color_t onColor()
Returns the current on state color.

## void on()
Sets the RGB LED to the on color.

##void setOffColor(_newColor_)
Sets the color of the _off_ state.
### Arguments
*  color_t _newColor_: New color for the RGB LED.

## color_t offColor()
Returns the current off state color.

## void off()
Sets the RGB LED to the off color.

## void toggle()
Toggles the RGB LED between the on and off colors.

* * *

# sbSpeaker
Class for controlling a speaker. This file includes the defines for the different notes as described in the [Arduino toneMelody](https://www.arduino.cc/en/Tutorial/ToneMelody) tutorial and the _pitches.h_ file.
## sbSpeaker(_pin_)
Creates the speaker object (constructor).
### Arguments
* uint8_t _pin_: the pin number of the speaker.

## void playNote(_freq_, _ms_, _restDur_)
Plays a note (tone) on the speaker at the given _freq_ and for _ms_ milliseconds and waits _restDur_ milliseconds.
### Arguments
* unsigned long _freq_: The frequency of the tone to play in Hertz.
* unsigned long _ms_: Duration of tone in milliseconds.
* unsigned long _restDur_: Duration to rest in milliseconds.

## void tone(_freq_)
Plays a tone on the speaker at the given _freq_. To turn off the tone, call the `noTone` method.
### Arguments
* unsigned _freq_: The frequency of the tone to play in Hertz.

## void tone(_freq_, _ms_)
Plays a tone on the speaker at the given _freq_ and for _ms_ milliseconds.
### Arguments
* unsigned long _freq_: The frequency of the tone to play in Hertz.
* unsigned long _ms_: Duration of tone in milliseconds.

## void noTone()
Turns off whatever tone is being played on the speaker.
## void beep(unsigned long _ms_)
Plays a "beep" (at 2000 Hz) for _ms_ milliseconds.
### Arguments
* unsigned long _ms_: Duration of beep in milliseconds. If _ms_ is not provided, 100 ms is used by default.

* * *

# sbDualMotors
Class for controlling two motors.
## sbMotors(_lia_, _lib_, _ria_, _rib_)
Creates a dual motor object (constructor).
### Arguments
* uint8_t _lia_: the pin number A of the left motor.
* uint8_t _lib_: the pin number B of the left motor.
* uint8_t _ria_: the pin number A of the right motor.
* uint8_t _rib_: the pin number B of the right  motor.

## void stop()
Stops both motors.
## void run(_leftSpeed_, _rightSpeed_)
Runs both motors in the direction and speed indicated by the arguments. If an argument is positive, then run the motor forward. If the number is negative, then run the motor backward. If the argument is 0, then stop the motor. The absolute value of the argument is the speed of the motor, range is 1 - 65,535.
### Arguments
* int16_t _leftSpeed_: the direction and speed for the left motor.
* int16_t _rightSpeed_: the direction and speed for the right motor.

* * *

# sbUSS
Class for using an ultrasonic sensor, compatible with an HC-SR04.
## sbUSS(_trigPin_, _echoPin_, _defaultDelay_)
Creates an ultrasound object (constructor).
### Arguments
* uint8_t _trigPin_: the pin number of the trigger for the HC-SR04.
* uint8_t _echoPin_: the pin number of the echo for the HC-SR04.
* unsigned long _defaultDelay_: the default delay for not detecting an echo in milliseconds.

## void setDelay(_newDelay_)
Sets the new default delay for not detecting an echo.
### Arguments
* unsigned long _newDelay_: the new default delay for not detecting an echo in milliseconds.

## float inches()
Returns the detected distance in inches. If no echo is received within the stored delay, then the value of `ULONG_MAX` is returned;
## float centimeters()
Returns the detected distance in centimeters. If no echo is received within the stored delay, then the value of `ULONG_MAX` is returned;

* * *

# sbTimer
Class for general purpose timers that are polled.
## sbTimer(_dur_, _autoRes_)
Creates a timer (constructor).
### Arguments
* unsigned long _dur_: the duration of the timer.
* bool _autoRes_: if _true_, then auto restart the timer. Default is _false_.

## void start(_dur_)
Starts a timer for the given duration. If no duration is given, the duration used in the constructor is used.
### Arguments
* unsigned long _dur_: the new duration (if not 0, which is the default).

## void stop()
Stops a timer.

## bool isActive()
Returns the state of a timer, true = active and false = stopped.

## bool timedOut()
Returns whether a timer has timed out, true = timed out and false = still active. When auto restart is true, the timer is automatically restarted.

## unsigned long remainingMs()
Returns the number of milliseconds before the timer times out.

* * *

# STEAMbot
Class for programming a STEAMbot robot. A STEAMbot object, called **sb** is automatically created. It has the following member objects:

* _runButton_ - an sbButton object for using the onboard RUN button.
* _stopButton_ - an sbButton object for using the onboard STOP button.
* _rgb_ - an sbRGB object for controlling the onboard RGB LED.
* _speaker_ - an sbSpeaker object for controlling the onboard speaker.
* _motors_ - an sbDualMotors object for controlling the motors of the STEAMbot robot.
* _ultrasonic_ - an sbUSS object for using the ultrasonic sensor.

* * *

# Bluetooth Communications

The STEAMbot has Bluetooth serial device called **sbBt**. If you plan to use it, you must have a `sbBT.begin(SB_BT_BAUD);` in your `setup` function. The API is the same as the `Serial` class of an Arduino.
