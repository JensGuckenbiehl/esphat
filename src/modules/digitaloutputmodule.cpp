#include "config/all.h"

#ifdef OUTPUT_SUPPORT

#include "digitaloutputmodule.h" 

#include "modulefactory.h"
REGISTER_MODULE("digitaloutput", DigitalOutputModule)

#include "config/all.h"

DigitalOutputModule::DigitalOutputModule() : _outputNumber(-1), 
        _statusTopic(""), _getTopic(""), _setTopic("") {
}

DigitalOutputModule::~DigitalOutputModule() {
}

void DigitalOutputModule::start() {
    if(_isValid() && mqttConnected()) {
        _registerTopics();
    }
}

void DigitalOutputModule::stop() {

}

String DigitalOutputModule::serialize() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.createObject();
    configJson["outputNumber"] = _outputNumber;
    configJson["statusTopic"] = _statusTopic;
    configJson["getTopic"] = _getTopic;
    configJson["setTopic"] = _setTopic;
    String output;
    configJson.printTo(output);
    return output;
}

void DigitalOutputModule::deserialize(const String& json) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.parseObject(json);
    if(configJson.containsKey("outputNumber")) {
        _outputNumber = configJson["outputNumber"];
    }
    if(configJson.containsKey("statusTopic")) {
        _statusTopic = configJson["statusTopic"].as<String>();
    }
    if(configJson.containsKey("getTopic")) {
        _getTopic = configJson["getTopic"].as<String>();
    }
    if(configJson.containsKey("setTopic")) {
        _setTopic = configJson["setTopic"].as<String>();
    }
}

void DigitalOutputModule::mqtt(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
        _registerTopics();
    }

    if (type == MQTT_MESSAGE_EVENT) {
        String t = topic;
        if(t == _getTopic) {
            mqttSendRaw(_statusTopic.c_str(), getOutput(_outputNumber) ? "1" : "0", true);
        } else if(t == _setTopic) {
            String p = payload;
            if(p == "0") {
                setOutput(_outputNumber, false);
            } else if(p == "1") {
                setOutput(_outputNumber, true);
            } else {
                setOutput(_outputNumber, !getOutput(_outputNumber));
            } 
            mqttSendRaw(_statusTopic.c_str(), getOutput(_outputNumber) ? "1" : "0", true);
        }
    }
}

bool DigitalOutputModule::_isValid() {
    return _outputNumber > 0 && _statusTopic.length() > 0 && _getTopic.length() > 0 && _setTopic.length() > 0;
}

void DigitalOutputModule::_registerTopics() {
    mqttSubscribeRaw(_getTopic.c_str());
    mqttSubscribeRaw(_setTopic.c_str());
}

#endif