#include "ledmodule.h" 

#include "modulefactory.h"
REGISTER_MODULE("led", LedModule)

#include "config/all.h"

LedModule::LedModule() : _gpio(-1), _mode(""), _led(NULL) {
    //registerLoop(std::bind(&LedModule::loop, this));
}

LedModule::~LedModule() {
    if(_led != NULL) {
        delete _led;
    } 
}

void LedModule::start() {
    if(_gpio != -1) {
        _led = new TTLED(_gpio, true);
    }
}

void LedModule::stop() {
    _gpio = -1;
    if(_led != NULL) {
        delete _led;
    } 
}

String LedModule::serialize() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.createObject();
    configJson["gpio"] = _gpio;
    configJson["mode"] = _mode;
    String output;
    configJson.printTo(output);
    return output;
}

void LedModule::deserialize(const String& json) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.parseObject(json);
    if(configJson.containsKey("gpio")) {
        _gpio = configJson["gpio"];
    }
    if(configJson.containsKey("mode")) {
        _mode = configJson["mode"].as<String>();
    }
}

void LedModule::loop() {
    if(_gpio == -1) {
        return;
    }
    
    if(_mode == "wifi") {
        if (wifiConnected()) {
            if (getWifiState() == WIFI_CONNECTED_AP) {
                _led->blinkAsync(800, 800);
            } else {
                _led->on();
            }
        } else {
            _led->blinkAsync(300, 300);
        }
    } else if(_mode == "mqtt") {
        if(mqttConnected()) {
            _led->on();
        } else {
            _led->off();
        }
    #ifdef OUTPUT_SUPPORT
    } else if(_mode == "outputMode") {
        if(manualModeOutput()) {
            _led->on();
        } else {
            _led->off();
        }
    } else if(_mode == "mqttOutputMode") {
        if(manualModeOutput()) {
            _led->on();
        } else if(!mqttConnected()) {
            _led->blinkAsync(800, 800);
        } else {
            _led->off();
        }
    #endif
    } else {
        _led->off();
    }

    _led->update();
}