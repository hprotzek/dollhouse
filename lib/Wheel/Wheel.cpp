#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include "Wheel.h"

Wheel::Wheel(uint16_t numTlc5974, Adafruit_TLC5947 *led) {
  _numTlc5974 = numTlc5974;
  _led = led;
  _interrupted = false;
}

void Wheel::loopColorWipe() {
  _colorWipe(4095, 0, 0, 100);
  _interruptDelay(200);
  _colorWipe(0, 4095, 0, 100);
  _interruptDelay(200);
  _colorWipe(0, 0, 4095, 100);
  _interruptDelay(200);
  _interrupted = false;
}

void Wheel::loopRainbowCycle() {
  _rainbowCycle(10);
  _interrupted = false;
}

void Wheel::interrupt() {
  _interrupted = true;
}

// Fill the dots one after the other with a color
void Wheel::_colorWipe(uint16_t r, uint16_t g, uint16_t b, uint8_t wait) {
  for(uint16_t i = 0; i < 8 * _numTlc5974 && !_interrupted; i++) {
      _led->setLED(i, r, g, b);
      _led->write();
      _interruptDelay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void Wheel::_rainbowCycle(uint8_t wait) {
  uint32_t i, j;

  for(j = 0; j < 4096 && !_interrupted; j++) { // 1 cycle of all colors on wheel
    for(i = 0; i < 8 * _numTlc5974 && !_interrupted; i++) {
      _wheel(i, ((i * 4096 / (8 * _numTlc5974)) + j) & 4095);
    }
    _led->write();
    _interruptDelay(wait);
  }
}

// Input a value 0 to 4095 to get a color value.
// The colours are a transition r - g - b - back to r.
void Wheel::_wheel(uint8_t ledn, uint16_t wheelPos) {
  if(wheelPos < 1365) {
    _led->setLED(ledn, 3*wheelPos, 4095 - 3*wheelPos, 0);
  } else if(wheelPos < 2731) {
    wheelPos -= 1365;
    _led->setLED(ledn, 4095 - 3*wheelPos, 0, 3*wheelPos);
  } else {
    wheelPos -= 2731;
    _led->setLED(ledn, 0, 3*wheelPos, 4095 - 3*wheelPos);
  }
}

void Wheel::_interruptDelay(unsigned long wait) {
  if (!_interrupted) {
    delay(wait);
  }
}
