#include "ledmodule.h"

#include "modulefactory.h"
REGISTER_MODULE("led", LedModule)

LedModule::LedModule() : _gpio(-1), _mode(""), _inverted(false), _led(NULL) {
  getVariables()->regsiterNotifier(this);
}

LedModule::~LedModule() {
  getVariables()->unregsiterNotifier(this);
  if (_led != NULL) {
    delete _led;
  }
}

void LedModule::start() {
  if (_gpio != -1) {
    _led = new OutputPin(_gpio, false, _inverted);
    update();
  }
}

void LedModule::stop() {
  _gpio = -1;
  if (_led != NULL) {
    delete _led;
  }
}

String LedModule::serialize() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJson = jsonBuffer.createObject();
  configJson["gpio"] = _gpio;
  configJson["mode"] = _mode;
  if (_inverted) {
    configJson["inverted"] = _inverted;
  }
  String output;
  configJson.printTo(output);
  return output;
}

void LedModule::deserialize(const JsonObject& configJson) {
  if (configJson.containsKey("gpio")) {
    _gpio = configJson["gpio"];
  }
  if (configJson.containsKey("mode")) {
    _mode = configJson["mode"].as<String>();
  }
  if (configJson.containsKey("inverted")) {
    _inverted = configJson["inverted"];
  } else {
    _inverted = false;
  }
}

void LedModule::handleKeyValueStoreChange(const String& key,
                                          const String& value,
                                          KeyValueStore* sender) {
  if (_led == NULL) {
    return;
  }
  if (_mode.equals("wifiState") && key.equals("wifiState")) {
    update();
  }
  if (_mode.equals("mqttState") && key.equals("mqttState")) {
    update();
  }
  if (_mode.equals("outputState") && key.equals("outputState")) {
    update();
  }
  if (_mode.equals("outputMqttState") &&
      (key.equals("outputState") || key.equals("mqttState"))) {
    update();
  }
}

void LedModule::update() {
  if (_mode.equals("wifiState")) {
    int state = getVariables()->get("wifiState", 0).toInt();
    if (state == WIFI_CONNECTED_AP) {
      _led->blink(800, 800);
    } else if (state == WIFI_CONNECTED_STA) {
      _led->on();
    } else {
      _led->blink(300, 300);
    }
  } else if (_mode.equals("mqttState")) {
    int state = getVariables()->get("mqttState", 0).toInt();
    if (state == 0) {
      _led->off();
    } else if (state == 1) {
      _led->on();
    }
  } else if (_mode.equals("outputState")) {
    int state = getVariables()->get("outputState", 0).toInt();
    if (state == 0) {
      _led->off();
    } else if (state == 1) {
      _led->on();
    }
  } else if (_mode.equals("outputMqttState")) {
    if (getVariables()->get("outputState", 0).toInt() == 1) {
      _led->on();
    } else if (getVariables()->get("mqttState", 0).toInt() == 0) {
      _led->blink(500, 500);
    } else {
      _led->off();
    }
  }
}

void LedModule::loop() {
  if (_led != NULL) {
    _led->update();
  }
}