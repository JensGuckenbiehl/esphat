#include "config/all.h"

#include "mqttlight.h"

#include "modulefactory.h"
REGISTER_MODULE("mqttlight", MqttLightModule)

#include "config/all.h"

MqttLightModule::MqttLightModule()
    : _outputNumber(-1),
      _statusTopic(""),
      _getTopic(""),
      _setTopic(""),
      _on(false),
      _statusBrightnessTopic(""),
      _getBrightnessTopic(""),
      _setBrightnessTopic(""),
      _brightness(100) {}

MqttLightModule::~MqttLightModule() {}

void MqttLightModule::start() {
  if (_outputNumber != -1) {
    _registerTopics();
    pinMode(_outputNumber, OUTPUT);
  }
}

void MqttLightModule::stop() {
  _unregisterTopics();
}

String MqttLightModule::serialize() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJson = jsonBuffer.createObject();
  configJson["outputNumber"] = _outputNumber;
  configJson["statusTopic"] = _statusTopic;
  configJson["getTopic"] = _getTopic;
  configJson["setTopic"] = _setTopic;
  configJson["statusBrightnessTopic"] = _statusBrightnessTopic;
  configJson["getBrightnessTopic"] = _getBrightnessTopic;
  configJson["setBrightnessTopic"] = _setBrightnessTopic;
  String output;
  configJson.printTo(output);
  return output;
}

void MqttLightModule::deserialize(const JsonObject& configJson) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJson = jsonBuffer.parseObject(json);
  if (configJson.containsKey("outputNumber")) {
    _outputNumber = configJson["outputNumber"];
  }
  if (configJson.containsKey("statusTopic")) {
    _statusTopic = configJson["statusTopic"].as<String>();
  }
  if (configJson.containsKey("getTopic")) {
    _getTopic = configJson["getTopic"].as<String>();
  }
  if (configJson.containsKey("setTopic")) {
    _setTopic = configJson["setTopic"].as<String>();
  }
  if (configJson.containsKey("statusBrightnessTopic")) {
    _statusTopic = configJson["statusBrightnessTopic"].as<String>();
  }
  if (configJson.containsKey("getBrightnessTopic")) {
    _getTopic = configJson["getBrightnessTopic"].as<String>();
  }
  if (configJson.containsKey("setBrightnessTopic")) {
    _setTopic = configJson["setBrightnessTopic"].as<String>();
  }
}

void MqttLightModule::mqtt(unsigned int type,
                           const char* topic,
                           const char* payload) {
  if (type == MQTT_CONNECT_EVENT) {
    _registerTopics();
  }

  if (type == MQTT_MESSAGE_EVENT) {
    String t = topic;
    if (t == _getTopic) {
      mqttSendRaw(_statusTopic.c_str(), _on ? "1" : "0", true);
    } else if (t == _setTopic) {
      String p = payload;
      if (p == "0") {
        analogWrite setOutput(_outputNumber, false);
      } else if (p == "1") {
        setOutput(_outputNumber, true);
      } else {
        setOutput(_outputNumber, !getOutput(_outputNumber));
      }
      mqttSendRaw(_statusTopic.c_str(), getOutput(_outputNumber) ? "1" : "0",
                  true);
    }
  }
}

void MqttLightModule::_registerTopics() {
  mqttSubscribeRaw(_getTopic.c_str());
  mqttSubscribeRaw(_setTopic.c_str());
  mqttSubscribeRaw(_getBrightnessTopic.c_str());
  mqttSubscribeRaw(_setBrightnessTopic.c_str());
}

void MqttLightModule::_unregisterTopics() {
  mqttUnsubscribeRaw(_getTopic.c_str());
  mqttUnsubscribeRaw(_setTopic.c_str());
  mqttUnsubscribeRaw(_getBrightnessTopic.c_str());
  mqttUnsubscribeRaw(_setBrightnessTopic.c_str());
}