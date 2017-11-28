#ifndef Room_h
#define Room_h

#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>

class Room {
  public:
    Room();

    void begin(String name, SX1509 *io, Adafruit_TLC5947 *led,
      int buttonPin, int ledPin,
      uint16_t red, uint16_t green, uint16_t blue, bool ledState);

    void loop();

  private:
    void _setColor(uint16_t red, uint16_t green, uint16_t blue);
    void _toggle();
    void _on();
    void _off();
    bool _isOn();
    bool _isOff();
    void _println(String msg);
    void _wheel(uint16_t wheelPos);

    SX1509 *_io;
    Adafruit_TLC5947 *_led;

    String _name;
    uint16_t _red, _green, _blue;
    uint32_t _wheelCounter;
    unsigned long _lastButtonPressed=0;
    int _buttonPin, _ledPin;
    bool _ledState, _triggered;
};

#endif
