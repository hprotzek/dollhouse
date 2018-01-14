#include <Adafruit_TLC5947.h>
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <Room.h>
#include <FS.h>
#include <FiniteStateMachine.h>
#include <Wheel.h>
#include <Button.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h"

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
unsigned long lastModeChange = 0;

// const int GREEN_PIN = D3;
// const int RED_PIN   = D4;

void setup_wifi();
void setup_networking();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnect();
WiFiClient espClient;
PubSubClient client(espClient);

//
// Json
//
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

void setup() {
  Serial.begin(115200);

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

  //
  // Setup SPIFFS
  //
  if (!SPIFFS.begin()) {
    Serial.println("failed to mount file system");
    SPIFFS.format();
    while(1) ;
  }

  setup_networking();

  Serial.println("Start initializing rooms");
  rooms[0].begin("wohnzimmer", &ioExt, &ledExt, &client, 0, 0);
  rooms[1].begin("kueche", &ioExt, &ledExt, &client, 1, 1);
  rooms[2].begin("kinder1", &ioExt, &ledExt, &client, 2, 2);
  rooms[3].begin("flur", &ioExt, &ledExt, &client, 3, 3);
  rooms[4].begin("kinder2", &ioExt, &ledExt, &client, 4, 4);
  rooms[5].begin("dach1", &ioExt, &ledExt, &client, 5, 5);
  rooms[6].begin("dach2", &ioExt, &ledExt, &client, 6, 6);
  //rooms[7].begin("reserve", &ioExt, &ledExt, 7, 7);
  Serial.println("Rooms start initializing successful");

  attachInterrupt(digitalPinToInterrupt(D3), nextMode, FALLING);

  // pinMode(RED_PIN, OUTPUT);
  // pinMode(GREEN_PIN, OUTPUT);
  // digitalWrite(GREEN_PIN, LOW);
  // digitalWrite(RED_PIN, LOW);
}

void setup_networking() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  //OTA SETUP
  ArduinoOTA.setPort(OTAport);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(SENSORNAME);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)OTApassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Starting");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!client.connected()) {
    reconnect();
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(SENSORNAME, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(dollhouse_command_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for(int n = 0; n < roomCount; n++) {
    if (strstr (topic, rooms[n].name)) {
      rooms[n].mqttCallback(payload, length);
    }
  }
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setup_wifi();
    return;
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  ArduinoOTA.handle();

  if (modeButton.uniquePress()){
   nextMode();
  }

  fsm.update();
}

void nextMode() {
  if(millis() - 500 > lastModeChange) {
    lastModeChange = millis();
    Serial.println("Next mode");
    wheel.interrupt();
    buttonPresses = ++buttonPresses % NUMBER_OF_STATES;
    switch (buttonPresses) {
      case 0: fsm.transitionTo(roomState); break;
      case 1: fsm.transitionTo(colorWipeState); break;
      case 2: fsm.transitionTo(rainbowCycleState); break;
    }
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
