#include "Adafruit_TLC5947.h"
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <Room.h>
#include "FS.h"
#include <FiniteStateMachine.h>
#include <Wheel.h>
#include <Button.h>

#define NUM_TLC5974 1
#define DATA        D5
#define CLOCK       D6
#define LATCH       D7

// led driver
Adafruit_TLC5947 ledExt = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);

// io shield
const byte SX1509_ADDRESS = 0x3E;
SX1509 ioExt;

// rooms
const int roomCount = 7;
Room* rooms = new Room[roomCount];

// wheelLoop
Wheel wheel = Wheel(NUM_TLC5974, &ledExt);

// fsm
void enterRoomLoop();
void roomLoop();
void exitRoomLoop();
void colorWipeLoop();
void rainbowCycleLoop();
void nextMode();
void button();
const byte NUMBER_OF_STATES = 3;
State roomState = State(enterRoomLoop, roomLoop, exitRoomLoop);
State colorWipeState = State(colorWipeLoop);
State rainbowCycleState = State(rainbowCycleLoop);
FSM fsm = FSM(roomState);
byte buttonPresses = 0;
Button modeButton = Button(D3, PULLUP);

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
  ioExt.debounceTime(32); // Set debounce time to 32 ms.
  Serial.println("IO shield start successful");

  attachInterrupt(digitalPinToInterrupt(D3), nextMode, FALLING);

  Serial.println("Start initializing rooms");
  rooms[0].begin("wohnzimmer", &ioExt, &ledExt, 0, 0);
  rooms[1].begin("kueche", &ioExt, &ledExt, 1, 1);
  rooms[2].begin("kinder1", &ioExt, &ledExt, 2, 2);
  rooms[3].begin("flur", &ioExt, &ledExt, 3, 3);
  rooms[4].begin("kinder2", &ioExt, &ledExt, 4, 4);
  rooms[5].begin("dach1", &ioExt, &ledExt, 5, 5);
  rooms[6].begin("dach2", &ioExt, &ledExt, 6, 6);
  //rooms[7].begin("reserve", &ioExt, &ledExt, 7, 7);
  Serial.println("Rooms start initializing successful");

  // pinMode(RED_PIN, OUTPUT);
  // pinMode(GREEN_PIN, OUTPUT);
  // digitalWrite(GREEN_PIN, LOW);
  // digitalWrite(RED_PIN, LOW);
}

void loop() {
  if (modeButton.uniquePress()){
   nextMode();
  }

  fsm.update();
}

void nextMode() {
  Serial.println("Next mode");
  wheel.interrupt();
  buttonPresses = ++buttonPresses % NUMBER_OF_STATES;
  switch (buttonPresses) {
    case 0: fsm.transitionTo(roomState); break;
    case 1: fsm.transitionTo(colorWipeState); break;
    case 2: fsm.transitionTo(rainbowCycleState); break;
  }
}

void enterRoomLoop() {
  for(int n = 0; n < roomCount; n++) {
    rooms[n].loadConfig();
  }
}

void roomLoop() {
  for(int n = 0; n < roomCount; n++) {
    rooms[n].loop();
  }
}

void exitRoomLoop() {
  for(int n = 0; n < roomCount; n++) {
    rooms[n].off();
  }
}

void colorWipeLoop() {
  wheel.loopColorWipe();
}

void rainbowCycleLoop() {
  wheel.loopRainbowCycle();
}
