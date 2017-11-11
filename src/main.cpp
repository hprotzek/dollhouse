#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <Room.h>

#define NUM_TLC5974 1
#define DATA        D5
#define CLOCK       D6
#define LATCH       D7

Adafruit_TLC5947 ledExt = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 ioExt; // Create an SX1509 object to be used throughout

Room wohn = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 1, 1, true);
Room kueche = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 2, 2, true);
Room kinder1 = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 3, 3, true);
Room flur = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 4, 4, true);
Room kinder2 = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 5, 5, true);
Room dach1 = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 6, 6, true);
Room dach2 = Room(&ioExt, &ledExt, random(0, 4095), random(0, 4095), random(0, 4095), 7, 7, true);
Room rooms[] = {wohn, kueche, kinder1, flur, kinder2, dach1, dach2};

void setup() {
  Serial.begin(9600);
  ledExt.begin();
  if (!ioExt.begin(SX1509_ADDRESS)) {
    Serial.println("IO start failed");
    while (1) ;
  }
  Serial.println("Dollhouse start");
}

void loop() {
  int i;
  for (i = 0; i < sizeof(rooms); i = i + 1) {
    rooms[i].loop();
  }
}
