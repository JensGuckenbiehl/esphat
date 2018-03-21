#include "module.h"

#include "config/all.h"

void Module::initialize(const String& name, const String& fileName, const String& type) {
    this->_name = name;
    this->_fileName = fileName;
    this->_type = type;

    File moduleFile = SPIFFS.open(_fileName, "r");
    if(moduleFile) {
        deserialize(moduleFile.readString());
        moduleFile.close();
    } else {
        Log.warning(F("[MODULES] Can't open module config file %s for module %s\n"), _fileName.c_str(), _name.c_str());
    }

    start();
}

const String& Module::getName() {
    return _name;
}

const String& Module::getFile() {
    return _fileName;
}

const String& Module::getType() {
    return _type;
}

String Module::getConfiguration() {
    return serialize();
}

void Module::setConfiguration(const String& config) {
    stop();
    deserialize(config);

    File moduleFile = SPIFFS.open(_fileName, "w");
    if(moduleFile) {
        moduleFile.print(serialize());
        moduleFile.close();
    } else {
        Log.error(F("[MODULES] Can't write module config file %s for module %s\n"), _fileName.c_str(), _name.c_str());
    }
    
    start();
}

void Module::deleteConfigurationFile() {
    if(SPIFFS.remove(_fileName)) {
        Log.warning(F("[MODULES] Deleted confguration file %s for module %s\n"), _fileName.c_str(), _name.c_str());
    } else {
        Log.error(F("[MODULES] Can't delete confguration file %s for module %s\n"), _fileName.c_str(), _name.c_str());
    }
}