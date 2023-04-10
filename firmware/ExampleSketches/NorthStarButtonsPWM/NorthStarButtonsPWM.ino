/*
   Preloaded example sketch for the Combine Reality Deck X Integrator Hub for Project North Star headsets
   Written by Noah Zerkin in 2020
    Do as you will with it, which I guess would make it MIT License? Let's go with that.
   Only stipulation: Don't use it as part of anything that will, by design, hurt anybody.
   I accept no liability for anything. If you live in a jurisdiction where that won't fly, don't run my poorly-written code.
   This sketch started out based on an example from the Sparkfun SX1509 library. Enjoy.
*/

#include <Wire.h> // I2C support library

/*Include SX1508 IO Expander Library. This is a modified version of the Sparkfun SX1509 Library.
   The SX1508 is the 8-pin varient, which has a different register map than the 16-pin SX1509.
*/
#include <SX1508.h>
#include "Keyboard.h"
#include <avr/pgmspace.h>

#define DEBUG 1

// Pin assignments
#define ARDUINO_INT_PIN 7 // SX1508 int output to D7
#define FAN 10          //5v fan circuit control (pwm of a MOSFET)
#define STORAGE 5       //Mass Storage subsystem power control
#define PORT_1 8        //USB 3 Port 1 power control
#define PORT_2 12       //USB 3 Port 2 power control
#define DISP_RST 11     //Display Driver Board reset. Integrator v1.01 and higher

// SX1508 I2C address
const byte SX1508_ADDRESS = 0x20;  // SX1508 I2C address
SX1508 buttonIO; // Create an SX1508 object for the Button Board

// Global variables:
bool buttonPressed = false; // Track button press in ISR
bool buttonsFailed = false; // Let's make sure the button board is actually there
const short BUTTON_DELAY = 200; //ms to wait for a dual press detection
const short RESET_DELAY = 2000; //ms to wait before executing dual press function

long buttonTimer[6] = {0, 0, 0, 0, 0, 0};   //place to store the time of each new button press
short portResetDuration = 500;              //time in ms to hold the reset state of the USB ports
long portResetTimeout = 0;                  //place to the time when the USB port power should be restored during a reset cycle
bool portResetting = false;                 //reset in progress boolean
byte timerMask = 0x00;                      //bits to denote whether a time recorded in the matching slot of buttonTimer is still relevant

bool serialPassthrough = false;

//Boolean to track if a ButtonFP function should be executed
bool buttonAction[6] = {0, 0, 0, 0, 0, 0};

//Array of function pointers for each button on the Integrator Button Board
typedef void (* FP)();
FP ButtonFP[6] = {&down, &right, &up, &left, &circle, &dot};

//Global bool to turn keycode output on or off, in case we want to switch to a different button function set
bool keyboardOutput = true;

//Array of keyboard keycodes to be sent for each of the buttons on the Integrator Button Board
byte keycode[6] = {KEY_DOWN_ARROW, KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_LEFT_ARROW, 0x5B, 0x5D};
// Keycodes can be ASCII chars or hex, or come from a limited set of preprocessor definitions defined here:
// https://github.com/arduino-libraries/Keyboard/blob/master/src/Keyboard.h
// Assuming that you want the buttons on the Integrator Button Board to send keyboard keys,
// this is someplace where you can keep and easily reassign them.
// By default, they are assigned to the arrow keys and '[' and ']', used to adjust IPD, eye elevation, and eye relief in the original
// Project North Star Unity example scene using v1 calibration.
// The array index for each keycode can coorespond to the the index of the pointer to the function from which you want to send it.
// That way we can easily change the keycodes in one place, or switch to an alternate keycode set.
// This can be expanded upon by using a reassignable pointer to this and other arrays of keycodes.
// Alternatively, we could add an additional dimension to this array.
//
// https://www.arduino.cc/reference/en/language/functions/usb/keyboard/
//
// Analog input pin for the thermistor
int thermistorPin = A5;
// Resistor value used in the voltage divider circuit
float resistorValue = 10000.0;
// Temperature coefficient of the thermistor
float betaValue = 3950.0;
// Room temperature (in Kelvin)
float roomTemperature = 292.15;
// Define the series resistance (in ohms) used in the voltage divider circuit
float seriesResistance = 4700.0;

void setup()
{
  pinMode(ARDUINO_INT_PIN, INPUT_PULLUP);
  pinMode(FAN, OUTPUT);
  pinMode(PORT_1, OUTPUT);
  pinMode(PORT_2, OUTPUT);
  pinMode(STORAGE, OUTPUT);
  pinMode(DISP_RST, OUTPUT);

  //Start with USB 3 ports and onboard mass storage subsystems powered up
  digitalWrite(PORT_2, HIGH);
  digitalWrite(PORT_1, HIGH);
  digitalWrite(STORAGE, HIGH);
  digitalWrite(DISP_RST, HIGH);

  // Initialize Serial port for debug output (or to relay serial i/o from display driver board with Integrator v1.01 or greater)
  Serial.begin(9600);
  // Initialize Serial1 port to communicate with display driver board using Integrator v1.01 or greater
  Serial1.begin(500000);

  // Call buttonIO.begin(<address>) to initialize the SX1508. If
  // it successfully communicates, it'll return 1.
  if (!buttonIO.begin(SX1508_ADDRESS))
  {
    Serial.println("Failed to communicate.");
    buttonsFailed = true;
    delay(500);
  }

  // buttonIO.configForNorthStarButtons();


  // Use buttonIO.pinMode(<pin>, <mode>) to set our button to an
  // input with internal pullup resistor activated:
  // buttonIO.writeByte(REG_INPUT_DISABLE, 0b11000000);
  // buttonIO.writeByte(REG_PULL_UP, 0b00111111);
  //  buttonIO.pinMode(SX1508_BUTTON_PIN, INPUT_PULLUP);
  if (!buttonsFailed) {
    for (byte i = 0; i < 6; i++) {
      buttonIO.pinMode(i, INPUT_PULLUP);
      delay(1);
    }

    // Use buttonIO.enableInterrupt(<pin>, <signal>) to enable an
    // interrupt on a pin. The <signal> variable can be either
    // FALLING, RISING, or CHANGE. Set it to falling, which will
    // mean the button was pressed:
    //  for (byte i = 0; i < 5; i++) {
    //    buttonIO.enableInterrupt(i, FALLING);
    //    delay(1);
    //  }

    // Seems I broke the enableInterrupt() function in Sparkfun's SX1509 library when modifying it for SX1508.
    // Writing to the registers inline for now. - Noah
    buttonIO.writeByte(0x09, 0b11000000);
    buttonIO.writeByte(0x0A, 0b00001111);
    buttonIO.writeByte(0x0B, 0b11111111);

    // The SX1508 has built-in debounce features, so a single
    // button-press doesn't accidentally create multiple ints.
    // Use buttonIO.debounceTime(<time_ms>) to set the GLOBAL SX1508
    // debounce time.
    // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
    buttonIO.debounceTime(32); // Set debounce time to 32 ms.

    // After configuring the debounce time, use
    //debouncePin(<pin>) to enable debounce on an input pin.
    for (byte i = 0; i < 6; i++) {
      buttonIO.debouncePin(i);
    }

    // Attach an Arduino interrupt to the interrupt pin (ARDUINO_INT_PIN). Call
    // the button function to set the buttonPressed flag whenever the pin goes from HIGH to
    // LOW.
    attachInterrupt(digitalPinToInterrupt(ARDUINO_INT_PIN),
                    button, FALLING);
  }
  digitalWrite(STORAGE, HIGH);
  digitalWrite(PORT_2, HIGH);
  digitalWrite(PORT_1, HIGH);
  Keyboard.begin();
  Keyboard.releaseAll();
}

void loop()
{
  // Update Fan Speed
  float temp = getTemp();
  int speed = getFanSpeed(temp);
  analogWrite(FAN,speed);
#if DEBUG
  Serial.print(temp);
  Serial.print(" ");
  Serial.println(speed);
#endif
  if (buttonPressed) // If the button() ISR was executed
  {
    buttonPressed = false; // Clear the buttonPressed flag
#if DEBUG
    Serial.println("Int triggered");
#endif
    // read buttonIO.interruptSource() find out which pin generated
    // an interrupt and clear the SX1508's interrupt output.
    byte intStatus = buttonIO.interruptSource(true);
    byte buttonStatus = 0xFF ^ buttonIO.readData(); // Read and invert button data byte, since buttons are active-low
    if (!buttonStatus) {
      Keyboard.releaseAll();
    }
#if DEBUG
    // Each bit in intStatus represents a single SX1508 IO.
    Serial.println("intStatus = " + String(intStatus, BIN) + " buttonStatus = " + String(buttonStatus, BIN));
#endif

    byte pressed = intStatus | buttonStatus;
    byte buttonNum = 0;
    byte numPressed = 0;
    for (byte i = 0; i < 6; i++) {
      if (intStatus & (1 << i)) {
        buttonNum = i;
#if DEBUG
        Serial.print("Button " + String(buttonNum) + " was ");
#endif
        if (buttonStatus & (1 << i)) {
#if DEBUG
          Serial.println("pressed.");
#endif

          //Record time that the button was pressed
          buttonTimer[i] = millis();

          //Mark the timer as "active"
          timerMask |= (1 << i);

          //Send associated keycode
          //(For D-Pad. Key sending functions for Circle and Dot buttons moved to timer delay to allow for dual press function)
          if (i < 4) {
            keyAction(i, true);
            buttonAction[i] = true;
          }
        }
        else {
          //Button triggered interrupt but isn't held... must've been released!
          Serial.println("released. ");
          keyAction(i, false);
          //"Deactivate" timer
          timerMask &= ~(1 << i);
#if DEBUG
          Serial.println("It was held for " + String(millis() - buttonTimer[i]) + "ms.");
#endif
        }
      }
      //How many buttons are held right now?
      if (buttonStatus & (1 << i)) {
        numPressed++;
      }
    }
#if DEBUG
    if (numPressed > 1) {
      Serial.println("There are " + String(numPressed) + " buttons being held.");
    }
    else {
      Serial.println();
    }
    Serial.println("TimerMask = " + String(timerMask, BIN));
#endif
  }
  timerActions();
  buttonActions();
  //Relay serial output from display driver if attached (v1.2 only)
  if (serialPassthrough) {
    while (Serial1.available()) {
      Serial.write(Serial1.read());
    }
    while (Serial.available()) {
      Serial1.write(Serial.read());
    }
  }
  else {
    while (Serial.available()) {
      char tempByte = Serial.read();
      switch (tempByte) {
        case 'r':
          portReset();
          break;
        case 'd':
          portDisable();
          break;
        case 'e':
          portEnable();
          break;
        case 'm':
          storageToggle();
          break;
        case 's':
          serialPassthrough = !serialPassthrough;
          break;
        case 'i':
          digitalWrite(DISP_RST, LOW);
          delay(10);
          digitalWrite(DISP_RST, HIGH);
          break;
         default:
          break;
      }
    }
    if(Serial1.available()){
      Serial1.flush();
    }
  }
}

// Calculate temperature of thermistor
float getTemp()
{
  delay(1000);
  // Read the analog input value from the thermistor pin
  int analogValue = analogRead(thermistorPin);

  // Convert the analog value to resistance (in ohms) using the voltage divider formula
  float resistance = seriesResistance / (1023.0 / analogValue - 1.0);

  // Calculate the temperature (in Kelvin) using the Steinhart-Hart equation
  float steinhart = log(resistance / resistorValue);     // ln(R/Ro)
  steinhart /= betaValue;                                // 1/B * ln(R/Ro)
  steinhart += 1.0 / roomTemperature;                    // + (1/To)
  steinhart = 1.0 / steinhart;                           // Invert
  float temperature = steinhart - 273.15;                // Convert to Celsius
  return temperature;
}

// Calculate fan speed against linear curve (tested PWM value range is 55-255)
int getFanSpeed(int temperature) {
  if (temperature < 35) {
    return 55;
  } if (temperature > 70) {
    return 255;
  } else {
    return map(temperature, 35, 70, 55, 160);
  }
}

void portReset() {
  digitalWrite(PORT_1, LOW);
  digitalWrite(PORT_2, LOW);
  portResetTimeout = millis() + portResetDuration;
  portResetting = true;
}

void storageToggle() {
  bool storageStatus = digitalRead(STORAGE);
  digitalWrite(STORAGE, !storageStatus);
  Serial.print("Mass storage turned ");
  if (storageStatus) {
    Serial.println("off.");
  }
  else {
    Serial.println("on.");
  }
}

void portDisable() {
  digitalWrite(PORT_1, LOW);
  digitalWrite(PORT_2, LOW);
#if DEBUG
  Serial.println("Ports Disabled");
#endif
}

void portEnable() {
  digitalWrite(PORT_1, HIGH);
  digitalWrite(PORT_2, HIGH);
#if DEBUG
  Serial.println("Ports Enabled");
#endif
}


//When called, this function either presses or releases a key from the keycode array
void keyAction(int key, bool pressing) {
  if (keyboardOutput) {
    if (pressing) {
      Keyboard.press(keycode[key]);
    }
    else {
      Keyboard.release(keycode[key]);
    }
  }
}

void timerActions() {
  if (timerMask) {                  //does a button have an active timer?
    long currentTime = millis();    //let's just read the time once. No need to keep consulting the hardware clock here
    if ((0b00010000 & timerMask) && !(0b00100000 & timerMask)) {    //Is button A held and button B isnt?
      if (currentTime - BUTTON_DELAY > buttonTimer[4]) {            //Has is been held longer than the time allocated to detect a dual-button press?
        keyAction(4, 1);                                 //Send the key!
        buttonAction[4] = true;
        timerMask &= (~(1 << 4));                                   //Kill the timer!
#if DEBUG
        Serial.println("TimerMask = " + String(timerMask, BIN));    //Tell the story!
#endif
      }
    }
    else {
      if ((0b00100000 & timerMask) && !(0b00010000 & timerMask)) {  //Same as above... yadda yadda yadda
        if (currentTime - BUTTON_DELAY > buttonTimer[5]) {
          keyAction(5, 1);
          buttonAction[5] = true;
          timerMask &= (~(1 << 5));
#if DEBUG
          Serial.println("TimerMask = " + String(timerMask, BIN));
#endif
        }
      }
      else {
        //Check if A and B have been held together for two seconds
        if ((0b00100000 & timerMask) && (0b00010000 & timerMask)) { //Are both buttons pressed? Separate comparisons because 0b00110000 & timerMask would evaluate to true for both OR one.
          long initiatedTime = currentTime - RESET_DELAY;                  //Let's count the dual press from when the first button press was detected
          if ((initiatedTime > buttonTimer[4]) && (initiatedTime > buttonTimer[5])) {
            portReset();
            timerMask &= (~(0b11 << 4));  //Kill timers for both A and B
#if DEBUG
            Serial.println("TimerMask = " + String(timerMask, BIN) + "Port Resetting");
#endif
          }
        }
      }
    }
  }

  if (portResetting && millis() > portResetTimeout) {
    digitalWrite(PORT_1, HIGH);
    digitalWrite(PORT_2, HIGH);
    portResetting = false;
#if DEBUG
    Serial.println("Port Reset complete");
#endif
  }
} //timerActions()

void buttonActions()
{
  for (int i = 0; i < 6; i++) {
    if (buttonAction[i]) {
      buttonAction[i] = false;
      ButtonFP[i]();
    }
  }
} //buttonActions()

void up() {
  //digitalWrite(STORAGE, !digitalRead(STORAGE));   //Toggle Storage Power
}

void down() {

}

void left() {
  //digitalWrite(PORT_1, !digitalRead(PORT_1));   //Toggle USB3 Port 1 Power
}

void right() {
  //digitalWrite(PORT_2, !digitalRead(PORT_2));   //Toggle USB3 Port 2 Power
}

void circle() {
/*
  if(serialPassthrough){
    serialPassthrough = false;
    Serial.println("Serial passthrough disabled");
  }
*/
}
 
void dot() {

}

// button() is an Arduino interrupt routine, called whenever
// the interrupt pin goes from HIGH to LOW.
void button()
{
  buttonPressed = true; // Set the buttonPressed flag to true
  // We can't do I2C communication in an Arduino ISR. The best
  // we can do is set a flag, to tell the loop() to check next
  // time through.
}
