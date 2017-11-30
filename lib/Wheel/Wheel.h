#ifndef Wheel_h
#define Wheel_h

#include "Arduino.h"
#include "Adafruit_TLC5947.h"

class Wheel {
  public:
    Wheel(uint16_t numTlc5974, Adafruit_TLC5947 *led);

    void loopColorWipe();
    void loopRainbowCycle();
    void interrupt();

  private:
    Adafruit_TLC5947 *_led;
    uint16_t _numTlc5974;
    bool _interrupted;

    void _colorWipe(uint16_t r, uint16_t g, uint16_t b, uint8_t wait);
    void _rainbowCycle(uint8_t wait);
    void _wheel(uint8_t ledn, uint16_t wheelPos);
    void _interruptDelay(unsigned long);
};

#endif
