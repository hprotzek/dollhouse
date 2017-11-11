
#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include "Room.h"
#include <Wire.h>
#include <SparkFunSX1509.h>

Room::Room(SX1509 *ioExt, Adafruit_TLC5947 *ledExt, uint16_t ledRedValue, uint16_t ledGreenValue, uint16_t ledBlueValue, int buttonPin, int ledPin, bool state) {

    _ioExt = ioExt;
    _ledExt = ledExt;

    _ledRedValue = ledRedValue;
    _ledGreenValue = ledGreenValue;
    _ledBlueValue = ledBlueValue;

    _buttonPin = buttonPin;
    _ledPin = ledPin;
    _state = state;

    _ioExt->pinMode(_buttonPin, INPUT_PULLUP);
    _ledExt->setLED(_ledPin, _ledRedValue, _ledGreenValue, _ledBlueValue);
    _ledExt->write();
}

void Room::loop() {
  if (_ioExt->digitalRead(_buttonPin) == LOW) {
    // If the button is pressed toggle the LED:
    _toggleLight();
    while (_ioExt->digitalRead(_buttonPin) == LOW)
      ; // Wait for button to release
  }
}

void Room::_toggleLight() {
  _state = !_state;
  if(_state) {
    _ledOn();
  } else {
    _ledOff();
  }
}

void Room::_ledOn() {
  _ledExt->setLED(_ledPin, _ledRedValue, _ledGreenValue, _ledBlueValue);
  _ledExt->write();
}

void Room::_ledOff() {
  _ledExt->setLED(_ledPin, 0, 0, 0);
  _ledExt->write();
}
