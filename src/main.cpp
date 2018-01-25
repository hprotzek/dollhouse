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
#define DATA D5
#define CLOCK D6
#define LATCH D7
#define TEMP_SENSOR_PIN A0
#define ERROR_LED_PIN D4
#define SUCCESS_LED_PIN D8

/* LED Shield */
Adafruit_TLC5947 ledExt = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);

/* IO Shield */
const byte SX1509_ADDRESS = 0x3E;
SX1509 ioExt;

/* Dollhouse Rooms */
const int roomCount = 7;
Room *rooms = new Room[roomCount];

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

void setup_wifi();
void setup_networking();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void reconnect();
bool processJson(char *message);
WiFiClient espClient;
PubSubClient client(espClient);

//
// Json
//
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

/* Temperature Sensor */
void tempLoop();
long previousMillis = 0;
float previousTemp = 0;
long interval = 60000;

void setup()
{
  Serial.begin(115200);

  pinMode(SUCCESS_LED_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  digitalWrite(SUCCESS_LED_PIN, LOW);
  digitalWrite(ERROR_LED_PIN, LOW);

  Serial.println("Start dollhouse");

  Serial.println("Start led driver");
  ledExt.begin();
  delay(5);
  Serial.println("Led driver start successful");

  Serial.println("Start io shield");
  if (!ioExt.begin(SX1509_ADDRESS))
  {
    Serial.println("IO shield start failed");
    digitalWrite(SUCCESS_LED_PIN, LOW);
    digitalWrite(ERROR_LED_PIN, HIGH);
    while (1)
      ;
  }
  delay(10);
  ioExt.debounceTime(32); // Set debounce time to 32 ms.
  Serial.println("IO shield start successful");

  //
  // Setup SPIFFS
  //
  if (!SPIFFS.begin())
  {
    Serial.println("failed to mount file system");
    SPIFFS.format();
    digitalWrite(SUCCESS_LED_PIN, LOW);
    digitalWrite(ERROR_LED_PIN, HIGH);
    while (1)
      ;
  }
  delay(10);

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

  digitalWrite(SUCCESS_LED_PIN, HIGH);
  digitalWrite(ERROR_LED_PIN, LOW);
}

void setup_networking()
{
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
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!client.connected())
  {
    reconnect();
  }
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(SENSORNAME, mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      client.subscribe(dollhouse_command_topic, 1);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++)
  {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message))
  {
    return;
  }

  for (int n = 0; n < roomCount; n++)
  {
    if (strstr(topic, rooms[n].name))
    {
      rooms[n].mqttCallback(payload, length);
      break;
    }
  }
}

bool processJson(char *message)
{
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(message);

  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return false;
  }

  if (root.containsKey("effect"))
  {
    if (root["effect"] == "Color Wipe")
    {
      buttonPresses = 1;
      fsm.transitionTo(colorWipeState);
    }
    else if (root["effect"] == "Rainbow Cycle")
    {
      buttonPresses = 2;
      fsm.transitionTo(rainbowCycleState);
    }
    else
    {
      buttonPresses = 0;
      fsm.transitionTo(roomState);
    }
  }

  return true;
}

void loop()
{

  if (WiFi.status() != WL_CONNECTED)
  {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setup_wifi();
    return;
  }

  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

  ArduinoOTA.handle();

  tempLoop();

  if (modeButton.uniquePress())
  {
    nextMode();
  }

  fsm.update();
}

void nextMode()
{
  if (millis() - 500 > lastModeChange)
  {
    lastModeChange = millis();
    Serial.println("Next mode");
    wheel.interrupt();
    buttonPresses = ++buttonPresses % NUMBER_OF_STATES;
    switch (buttonPresses)
    {
    case 0:
      fsm.transitionTo(roomState);
      break;
    case 1:
      fsm.transitionTo(colorWipeState);
      break;
    case 2:
      fsm.transitionTo(rainbowCycleState);
      break;
    }
  }
}

void enterRoomLoop()
{
  for (int n = 0; n < roomCount; n++)
  {
    rooms[n].loadConfig();
  }
}

void roomLoop()
{
  for (int n = 0; n < roomCount; n++)
  {
    rooms[n].loop();
  }
}

void exitRoomLoop()
{
  for (int n = 0; n < roomCount; n++)
  {
    rooms[n].off();
  }
}

void colorWipeLoop()
{
  wheel.loopColorWipe();
}

void rainbowCycleLoop()
{
  wheel.loopRainbowCycle();
}

/* Temperature Sensor */
void tempLoop()
{
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;   
 
    int reading = analogRead(TEMP_SENSOR_PIN);
    float voltage = reading * 3.3;
    voltage /= 1024.0;
    float currentTemp = (voltage - 0.5) * 100; 
    Serial.print(currentTemp);
    Serial.println(" degrees C");
    
    if(currentTemp != previousTemp) {
      previousTemp = currentTemp;
      char tempStr[10];
      dtostrf(currentTemp, 6, 1, tempStr);
      Serial.println("Send temp over mqtt");
      client.publish(dollhouse_temp_topic, tempStr);
    }
  }
}
