#include "module.h"

void Module::initialize(const String& name,
                        const String& fileName,
                        const String& type) {
  this->_name = name;
  this->_fileName = fileName;
  this->_type = type;

  File moduleFile = SPIFFS.open(_fileName, "r");
  if (moduleFile) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.parseObject(moduleFile);
    deserialize(configJson);
    moduleFile.close();
  } else {
    Terminal::logLevel(
        Loglevel::WARNING,
        F("[MODULES] Can't open module config file %s for module %s"),
        _fileName.c_str(), _name.c_str());
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJson = jsonBuffer.parseObject(config);
  deserialize(configJson);

  File moduleFile = SPIFFS.open(_fileName, "w");
  if (moduleFile) {
    moduleFile.print(serialize());
    moduleFile.close();
  } else {
    Terminal::logLevel(
        Loglevel::ERROR,
        F("[MODULES] Can't write module config file %s for module %s"),
        _fileName.c_str(), _name.c_str());
  }

  start();
}

void Module::deleteConfigurationFile() {
  if (SPIFFS.remove(_fileName)) {
    Terminal::logLevel(
        Loglevel::WARNING,
        F("[MODULES] Deleted confguration file %s for module %s"),
        _fileName.c_str(), _name.c_str());
  } else {
    Terminal::logLevel(
        Loglevel::ERROR,
        F("[MODULES] Can't delete confguration file %s for module %s"),
        _fileName.c_str(), _name.c_str());
  }
}