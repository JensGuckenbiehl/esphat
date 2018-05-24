#ifndef MQTTLIGHT_H
#define MQTTLIGHT_H

#include "module.h"

class MqttLightModule : public Module {
 private:
  u8_t _outputNumber;

  String _statusTopic;
  String _setTopic;
  String _getTopic;
  bool _on;

  String _statusBrightnessTopic;
  String _setBrightnessTopic;
  String _getBrightnessTopic;
  u8_t _brightness;

  void _registerTopics();
  void _unregisterTopics();

 public:
  MqttLightModule();
  ~MqttLightModule();
  void start();
  void stop();
  void mqtt(unsigned int type, const char* topic, const char* payload);

 protected:
  String serialize();
  void deserialize(const JsonObject& configJson);
};

#endif  // MQTTLIGHT_H