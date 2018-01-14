#ifndef Room_h
#define Room_h

#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <PubSubClient.h>

struct rgb {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
};

class Room {
  public:
    Room();

    void begin(String name, SX1509 *io, Adafruit_TLC5947 *led, PubSubClient *mqtt,
      int buttonPin, int ledPin);

    void loop();
    void loadConfig();
    void on();
    void off();
    bool getState();
    struct rgb getColor();
    void setColor(uint16_t red, uint16_t green, uint16_t blue);

  private:
    void _toggle();
    bool _isOn();
    bool _isOff();
    void _println(String msg);
    void _wheel(uint16_t wheelPos);
    void _saveConfig();
    bool _loadConfig();
    void _sendState();

    SX1509 *_io;
    Adafruit_TLC5947 *_led;
    PubSubClient *_mqtt;

    String _name;
    uint16_t _red, _green, _blue;
    uint32_t _wheelCounter;
    unsigned long _lastButtonPressed=0;
    int _buttonPin, _ledPin;
    bool _ledState, _triggered;
};

#endif
