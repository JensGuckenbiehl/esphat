/*

MODULE MANAGER MODULE

*/

#include "modules/module.h"
#include "modules/modulefactory.h"
#include <ArduinoJson.h>
#include <vector>

std::vector<std::shared_ptr<Module>> _modules;

bool _createModule(JsonObject& module) {
    String name = module["name"];
    String file = module["file"];
    String type = module["type"];

    Log.notice(F("[MODULES] Create module. Name = \"%s\", type = \"%s\", file = \"%s\"\n"), name.c_str(), type.c_str(), file.c_str());

    if(name.length() == 0 || file.length() == 0 || type.length() == 0) {
        Log.warning(F("[MODULES] Can't create module. Not all needed values available\n"));
        return false;
    }

    auto moduleInstance = ModuleFactory::Instance()->Create(type);
    if(moduleInstance) {
        moduleInstance->initialize(name, file, type);
        _modules.push_back(std::move(moduleInstance));
        Log.notice(F("[MODULES] Created module\n"));
        return true;
    } 

    Log.warning(F("[MODULES] Can't create module. Invalid type\n"));
    return false;
}

String _getJson() {
    DynamicJsonBuffer jsonBuffer;
    JsonArray& root = jsonBuffer.createArray();

    for(auto& module : _modules) {
        JsonObject& moduleJson = jsonBuffer.createObject();
        moduleJson["name"] = module->getName();
        moduleJson["type"] = module->getType();
        moduleJson["file"] = module->getFile();
        root.add(moduleJson);
    }

    String output;
    root.printTo(output);

    return output;
}

void _saveModulesFile() {
    Log.notice(F("[MODULES] Save modules\n")); 
    File modulesFile = SPIFFS.open(MODULES_FILE, "w");
    if(modulesFile) {
        modulesFile.print(_getJson());
        modulesFile.close();
    } else {
        Log.error(F("[MODULES] Can't write modules file %s\n"), MODULES_FILE);
    }
}

void _loadAndInitializeModules() {
    SPIFFS.begin();
    File modulesFile = SPIFFS.open(MODULES_FILE, "r");
    if(!modulesFile) {
        Log.warning(F("[MODULES] Can't open modules file %s\n"), MODULES_FILE);
        return;
    }

    // Let ArduinoJson read directly from File
    DynamicJsonBuffer jb;
    JsonArray& modules = jb.parseArray(modulesFile);

    if (!modules.success()) {
        Log.warning(F("[MODULES] Can't parse modules file %s\n"), MODULES_FILE);
        modulesFile.close();
        return;
    } 

    for (auto& module : modules) {
        if(module.is<JsonObject>()) {
            _createModule(module.as<JsonObject>());
        }
    }

    modulesFile.close();
}

String _getMqttTopicModuleName(String topicKey, String topic) {
    int position = topic.indexOf("+");
    if (position == -1) return String();
    String start =topic.substring(0, position);
    String end = topic.substring(position + 1);

    if (topicKey.startsWith(start) && topicKey.endsWith(end)) {
        topicKey.replace(start, "");
        topicKey.replace(end, "");
    } else {
        topicKey = String();
    }

    return topicKey;

}

void _modulesMQTTCallback(unsigned int type, const char * topic, const char * payload) {

    if (type == MQTT_CONNECT_EVENT) {
        mqttSubscribe(MOUDULES_ADD_MQTT_TOPIC);
        mqttSubscribe(MOUDULES_GET_MQTT_TOPIC);
        mqttSubscribe(MOUDULE_GET_MQTT_TOPIC);
        mqttSubscribe(MOUDULE_SET_MQTT_TOPIC);
        mqttSubscribe(MOUDULE_DELETE_MQTT_TOPIC);
    }

    if (type == MQTT_MESSAGE_EVENT) {

        // Match topic
        String t = mqttMagnitude((char *) topic);
        if (t == MOUDULES_ADD_MQTT_TOPIC) {
            Log.notice(F("[MODULES] Add module\n")); 
        
            DynamicJsonBuffer jsonBuffer;
            JsonObject& module = jsonBuffer.parseObject(payload);
            if(_createModule(module)) {
                _saveModulesFile();
            }
        } else if (t == MOUDULES_GET_MQTT_TOPIC) {
            Log.notice(F("[MODULES] Get modules\n")); 
            mqttSend(MOUDULES_MQTT_TOPIC, _getJson().c_str());
        } else if(_getMqttTopicModuleName(t, MOUDULE_GET_MQTT_TOPIC).length() != 0) {
            String moduleName = _getMqttTopicModuleName(t, MOUDULE_GET_MQTT_TOPIC);
            Log.notice(F("[MODULES] Get module \"%s\"\n"), moduleName.c_str()); 
            for(auto& module : _modules) {
                if(module->getName() == moduleName) {
                    String answerTopic = MOUDULE_MQTT_TOPIC;
                    answerTopic.replace("+", moduleName);
                    mqttSend(answerTopic.c_str(), module->getConfiguration().c_str());
                }
            }
        } else if(_getMqttTopicModuleName(t, MOUDULE_SET_MQTT_TOPIC).length() != 0) {
            String moduleName = _getMqttTopicModuleName(t, MOUDULE_SET_MQTT_TOPIC);
            Log.notice(F("[MODULES] Set module \"%s\"\n"), moduleName.c_str()); 
            for(auto& module : _modules) {
                if(module->getName() == moduleName) {
                    module->setConfiguration(payload);
                }
            }
        } else if(_getMqttTopicModuleName(t, MOUDULE_DELETE_MQTT_TOPIC).length() != 0) {
            String moduleName = _getMqttTopicModuleName(t, MOUDULE_DELETE_MQTT_TOPIC);
            Log.notice(F("[MODULES] Delete module \"%s\"\n"), moduleName.c_str()); 
            int deleteItemIndex = 0;
            bool deleteItem = false;
            for(auto& module : _modules) {
                if(module->getName() == moduleName) {
                    module->stop();
                    module->deleteConfigurationFile();
                    deleteItem = true;
                    break;
                }
                deleteItemIndex++;
            }
            if(deleteItem) {
                _modules.erase(_modules.begin() + deleteItemIndex);
                _saveModulesFile();
            }
        }
            
    }

    for(auto& module : _modules) {
        module->mqtt(type, topic, payload);    
    }

}

void modulesSetup() {
    Log.notice(F("[MODULES] Setup modules\n"));

    _loadAndInitializeModules();

    mqttRegister(_modulesMQTTCallback);
    registerLoop(_loop);

}

void _loop() {
    for(auto& module : _modules) {
        module->loop();    
    }
}