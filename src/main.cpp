#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>


#define NUM_TLC5974 1

#define data   D5
#define clock   D6
#define latch   D7

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock, data, latch);

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 io; // Create an SX1509 object to be used throughout

// Call io.pinMode(<pin>, <mode>) to set any SX1509 pin as
// either an INPUT, OUTPUT, INPUT_PULLUP, or ANALOG_OUTPUT
const int SX1509_BTN_PIN = 7;
const int SX1509_LED_PIN = 15;

bool light = true;

void setup() {
  Serial.begin(9600);
  Serial.println("TLC5974 test");

  tlc.begin();
  if (!io.begin(SX1509_ADDRESS)) {
    while (1) ;
  }

  io.pinMode(SX1509_LED_PIN, OUTPUT);
  io.pinMode(SX1509_BTN_PIN, INPUT_PULLUP);

  // Blink the LED a few times before we start:
  for (int i=0; i<5; i++)
  {
    // Use io.digitalWrite(<pin>, <LOW | HIGH>) to set an
    // SX1509 pin either HIGH or LOW:
    io.digitalWrite(SX1509_LED_PIN, HIGH);
    delay(100);
    io.digitalWrite(SX1509_LED_PIN, LOW);
    delay(100);
  }
}


void toggleLight(uint16_t i) {
  light = !light;
  if(light) {
    tlc.setLED(i, random(0, 4095), random(0, 4095), random(0, 4095));
    tlc.write();
  } else {
    tlc.setLED(i, 0, 0, 0);
    tlc.write();
  }
}

void loop() {
  // Use io.digitalRead() to check if an SX1509 input I/O is
  // either LOW or HIGH.
  if (io.digitalRead(SX1509_BTN_PIN) == LOW) {
    // If the button is pressed toggle the LED:
    toggleLight(0);
    while (io.digitalRead(SX1509_BTN_PIN) == LOW)
      ; // Wait for button to release
  }
}
