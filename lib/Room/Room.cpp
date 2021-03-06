#include "Arduino.h"
#include "Adafruit_TLC5947.h"
#include "Room.h"
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <ArduinoJson.h>
#include "FS.h"

const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

Room::Room(){
}

void Room::begin(const char *nameArg, SX1509 *io, Adafruit_TLC5947 *led, PubSubClient *mqtt,
  int buttonPin, int ledPin) {

  name = nameArg;

  _io = io;
  _led = led;
  _mqtt = mqtt;

  _buttonPin = buttonPin;
  _ledPin = ledPin;

  _io->pinMode(_buttonPin, INPUT_PULLUP);
  _io->debouncePin(_buttonPin);

  _println("mounting FS...");
  loadConfig();
  _sendState();

  _println("successful initialized");
}

void Room::loop() {
  if (_io->digitalRead(_buttonPin) == LOW && !_triggered) {
    _println("button pressed");
    _lastButtonPressed = millis();
    _triggered = true;

    while (_io->digitalRead(_buttonPin) == LOW) {
      delay(5);
      if(millis() - 1000 > _lastButtonPressed) {
        if (_isOff()) {
          on();
        }
        if(_wheelCounter<4096) {
          _wheel((4096 + _wheelCounter) & 4095);
          _led->write();
          _wheelCounter++;
        } else {
          _wheelCounter = 0;
        }
      }
    }

    if (_triggered) {
      if (millis() - 1000 < _lastButtonPressed) {
        _toggle();
      }
      _saveConfig();
      _sendState();
      _triggered = false;
    }
  }
}

void Room::loadConfig() {
  if (!_loadConfig()) {
    _println("failed to load config");
    _red = random(0, 4095);
    _green = random(0, 4095);
    _blue = random(0, 4095);
    _ledState = false;
    _wheelCounter = 0;
  } else {
    _println("config loaded");
  }

  if (_ledState) {
    on();
  } else {
    off();
  }
}

void Room::_toggle() {
  if(_isOn()) {
    off();
  } else {
    on();
  }
}

bool Room::_isOff() {
  return !_ledState;
}

bool Room::_isOn() {
  return _ledState;
}

void Room::on() {
  _ledState = true;
  _led->setLED(_ledPin, _red, _green, _blue);
  _led->write();
  _println("light on");
}

void Room::off() {
  _ledState = false;
  _led->setLED(_ledPin, 0, 0, 0);
  _led->write();
  _println("light off");
}

bool Room::getState() {
  return _ledState;
}

void Room::_sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["state"] = (_ledState) ? "ON" : "OFF";
  JsonObject& color = root.createNestedObject("color");
  color["r"] = map(_red, 0, 4096, 0, 255);
  color["g"] = map(_green, 0, 4096, 0, 255);
  color["b"] = map(_blue, 0, 4096, 0, 255);

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  char targetStr[50] = {0};
  strcat(targetStr, "home/dollhouse/");
  strcat(targetStr, name);
  strcat(targetStr, "/state");
  _println(targetStr);
  _mqtt->publish(targetStr, buffer, false);
}

void Room::mqttCallback(byte* payload, unsigned int length) {
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!_processJson(message)) {
    return;
  }

  _sendState();
}

/********************************** START PROCESS JSON*****************************************/
bool Room::_processJson(char* message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }

  if (root.containsKey("state")) {
    if (strcmp(root["state"], "ON") == 0) {
      on();
    } else if (strcmp(root["state"], "OFF") == 0) {
      off();
    }
  }

  if (root.containsKey("color")) {
    setColor(
      map(root["color"]["r"], 0, 255, 0, 4096),
      map(root["color"]["g"], 0, 255, 0, 4096),
      map(root["color"]["b"], 0, 255, 0, 4096));
  }

  return true;
}

void Room::_println(String msg) {
  Serial.print(String(name));
  Serial.print(" ");
  Serial.println(msg);
}

void Room::setColor(uint16_t red, uint16_t green, uint16_t blue) {
  _red = red;
  _green = green;
  _blue = blue;
  _led->setLED(_ledPin, _red, _green, _blue);
}

void Room::_setLED() {
  uint16_t colors[3];
  
  _led->setLED(_ledPin, _red, _green, _blue);
}

struct rgb Room::getColor() {
  struct rgb color;
  color.red = _red;
  color.green = _green;
  color.blue = _blue;
  return color;
}

void Room::_wheel(uint16_t wheelPos) {
  if(wheelPos < 1365) {
    setColor(3*wheelPos, 4095 - 3*wheelPos, 0);
  } else if(wheelPos < 2731) {
    wheelPos -= 1365;
    setColor(4095 - 3*wheelPos, 0, 3*wheelPos);
  } else {
    wheelPos -= 2731;
    setColor(0, 3*wheelPos, 4095 - 3*wheelPos);
  }
}

void Room::_saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["red"] = _red;
  json["green"] = _green;
  json["blue"] = _blue;
  json["state"] = _ledState;
  json["wheel"] = _wheelCounter;

  File configFile = SPIFFS.open("/config_"+String(name)+".json", "w");
  if (!configFile) {
    _println("Failed to open config file for writing");
  }

  json.printTo(configFile);
  configFile.close();
}

bool Room::_loadConfig() {
  File configFile = SPIFFS.open("/config_"+String(name)+".json", "r");
  if (!configFile) {
    _println("failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    _println("config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  configFile.close();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    _println("failed to parse config file");
    return false;
  }

  _red = json["red"];
  _green = json["green"];
  _blue = json["blue"];
  _ledState = json["state"];
  _wheelCounter = json["wheel"];
  _println("fconfig loaded successfully");
  return true;
}
