#ifndef Room_h
#define Room_h

#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>

class Room {
  public:
    Room(SX1509 *ioExt, Adafruit_TLC5947 *ledExt,
      uint16_t ledRedValue, uint16_t ledGreenValue, uint16_t ledBlueValue,
      int buttonPin, int ledPin, bool state);

    void loop();

  private:
    void _toggleLight();
    void _setLed(int ledPin, uint16_t ledRedValue, uint16_t ledGreenValue, uint16_t ledBlueValue);
    void _ledOn();
    void _ledOff();

    SX1509 *_ioExt;
    Adafruit_TLC5947 *_ledExt;

    uint16_t _ledRedValue, _ledGreenValue, _ledBlueValue;
    int _buttonPin, _ledPin;
    bool _state;
};

#endif
