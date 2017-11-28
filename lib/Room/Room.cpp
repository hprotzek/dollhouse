
#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include "Room.h"
#include <Wire.h>
#include <SparkFunSX1509.h>

Room::Room(){
  _wheelCounter = 0;
}

void Room::begin(String name, SX1509 *io, Adafruit_TLC5947 *led,
  int buttonPin, int ledPin, uint16_t red, uint16_t green, uint16_t blue, bool ledState) {

  _name = name;

  _io = io;
  _led = led;

  _red = red;
  _green = green;
  _blue = blue;

  _buttonPin = buttonPin;
  _ledPin = ledPin;
  _ledState = ledState;

  _io->pinMode(_buttonPin, INPUT_PULLUP);
  _io->debouncePin(_buttonPin);

  if (_ledState) {
    _on();
  } else {
    _off();
  }

  _println("successful initialized");
}

void Room::loop() {
  if (_io->digitalRead(_buttonPin) == LOW && !_triggered) {
    _println("button pressed");
    _lastButtonPressed = millis();
    _triggered = true;

    while (_io->digitalRead(_buttonPin) == LOW) {
      delay(10);
      if(millis() - 1000 > _lastButtonPressed) {
        if (_isOff()) {
          _on();
        }
        if(_wheelCounter<4096) {
          _wheel((4096 + _wheelCounter) & 4095);
          _led->write();
          _wheelCounter++;
        } else {
          _wheelCounter = 0;
        }
      }
    }

    if (_triggered) {
      if (millis() - 1000 < _lastButtonPressed) {
        _toggle();
      }
      _triggered = false;
    }
  }
}

void Room::_toggle() {
  if(_isOn()) {
    _off();
  } else {
    _on();
  }
}

bool Room::_isOff() {
  return !_ledState;
}

bool Room::_isOn() {
  return _ledState;
}

void Room::_on() {
  _ledState = true;
  _led->setLED(_ledPin, _red, _green, _blue);
  _led->write();
  _println("light on");
}

void Room::_off() {
  _ledState = false;
  _led->setLED(_ledPin, 0, 0, 0);
  _led->write();
  _println("light off");
}

void Room::_println(String msg) {
  Serial.print(_name);
  Serial.print(" ");
  Serial.println(msg);
}

void Room::_setColor(uint16_t red, uint16_t green, uint16_t blue) {
  _red = red;
  _green = green;
  _blue = blue;
  _led->setLED(_ledPin, _red, _green, _blue);
}

void Room::_wheel(uint16_t wheelPos) {
  if(wheelPos < 1365) {
    _setColor(3*wheelPos, 4095 - 3*wheelPos, 0);
  } else if(wheelPos < 2731) {
    wheelPos -= 1365;
    _setColor(4095 - 3*wheelPos, 0, 3*wheelPos);
  } else {
    wheelPos -= 2731;
    _setColor(0, 3*wheelPos, 4095 - 3*wheelPos);
  }
}
