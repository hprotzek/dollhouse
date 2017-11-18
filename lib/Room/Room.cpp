
#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include "Room.h"
#include <Wire.h>
#include <SparkFunSX1509.h>

Room::Room(){
  _wheelCounter = 0;
}

void Room::begin(String name, SX1509 *ioExt, Adafruit_TLC5947 *ledExt, uint16_t ledRedValue, uint16_t ledGreenValue, uint16_t ledBlueValue, int buttonPin, int ledPin, bool state) {
  _name = name;

  _ioExt = ioExt;
  _ledExt = ledExt;

  _ledRedValue = ledRedValue;
  _ledGreenValue = ledGreenValue;
  _ledBlueValue = ledBlueValue;

  _buttonPin = buttonPin;
  _ledPin = ledPin;
  _state = state;

  _ioExt->pinMode(_buttonPin, INPUT_PULLUP);
  _ioExt->debouncePin(_buttonPin);
  _ledExt->setLED(_ledPin, _ledRedValue, _ledGreenValue, _ledBlueValue);
  _ledExt->write();

  _println("successful initialized");
}

void Room::loop() {
  if (_ioExt->digitalRead(_buttonPin) == LOW) {
    _println("button pressed");
    _toggleLight();
    while (_ioExt->digitalRead(_buttonPin) == LOW) {
      if(_wheelCounter<4096) {
        _wheel((4096 + _wheelCounter) & 4095);
        _ledExt->write();
        _wheelCounter++;
      } else {
        _wheelCounter = 0;
      }
    }
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
  _println("light on");
}

void Room::_ledOff() {
  _ledExt->setLED(_ledPin, 0, 0, 0);
  _ledExt->write();
  _println("light off");
}

void Room::_println(String msg) {
  Serial.print(_name);
  Serial.print(" ");
  Serial.println(msg);
}

void Room::_wheel(uint16_t wheelPos) {
  if(wheelPos < 1365) {
    _ledExt->setLED(_ledPin, 3*wheelPos, 4095 - 3*wheelPos, 0);
  } else if(wheelPos < 2731) {
    wheelPos -= 1365;
    _ledExt->setLED(_ledPin, 4095 - 3*wheelPos, 0, 3*wheelPos);
  } else {
    wheelPos -= 2731;
    _ledExt->setLED(_ledPin, 0, 3*wheelPos, 4095 - 3*wheelPos);
  }
}
