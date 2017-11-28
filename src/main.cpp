#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <Room.h>
#include "FS.h"

#define NUM_TLC5974 1
#define DATA        D5
#define CLOCK       D6
#define LATCH       D7
// #define DATA        4
// #define CLOCK       5
// #define LATCH       6

Adafruit_TLC5947 ledExt = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);

// // SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 ioExt; // Create an SX1509 object to be used throughout

const int roomCount = 7;
Room* rooms = new Room[roomCount];

// const int GREEN_PIN = D3;
// const int RED_PIN   = D4;

void setup() {
  Serial.begin(9600);

  Serial.println("Start dollhouse");

  Serial.println("Start led driver");
  ledExt.begin();
  Serial.println("Led driver start successful");

  Serial.println("Start io shield");
  if (!ioExt.begin(SX1509_ADDRESS)) {
    Serial.println("IO shield start failed");
    // digitalWrite(GREEN_PIN, LOW);
    // digitalWrite(RED_PIN, HIGH);
    while (1) ;
  }
  ioExt.debounceTime(32);
  Serial.println("IO shield start successful");

  // Serial.println("Mounting FS...");
  //
  // if (!SPIFFS.begin()) {
  //   Serial.println("Failed to mount file system");
  //   return;
  // }
  //SPIFFS.format();

  Serial.println("Start initializing rooms");
  rooms[0].begin("wohnzimmer", &ioExt, &ledExt, 0, 0, random(0, 4095), random(0, 4095), random(0, 4095), false);
  rooms[1].begin("küche", &ioExt, &ledExt, 1, 1, random(0, 4095), random(0, 4095), random(0, 4095), false);
  rooms[2].begin("kinder1", &ioExt, &ledExt, 2, 2, random(0, 4095), random(0, 4095), random(0, 4095), false);
  rooms[3].begin("flur", &ioExt, &ledExt, 3, 3, random(0, 4095), random(0, 4095), random(0, 4095), false);
  rooms[4].begin("kinder2", &ioExt, &ledExt, 4, 4, random(0, 4095), random(0, 4095), random(0, 4095), false);
  rooms[5].begin("dach1", &ioExt, &ledExt, 5, 5, random(0, 4095), random(0, 4095), random(0, 4095), false);
  rooms[6].begin("dach2", &ioExt, &ledExt, 6, 6, random(0, 4095), random(0, 4095), random(0, 4095), false);
  Serial.println("Rooms start initializing successful");

  // pinMode(RED_PIN, OUTPUT);
  // pinMode(GREEN_PIN, OUTPUT);
  // digitalWrite(GREEN_PIN, LOW);
  // digitalWrite(RED_PIN, LOW);
}

void loop() {
  // digitalWrite(GREEN_PIN, HIGH);
  // digitalWrite(RED_PIN, LOW);
  for(int n = 0; n < roomCount; n++) {
     rooms[n].loop();
 }
}
