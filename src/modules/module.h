#ifndef MODULE_H
#define MODULE_H

#include <Arduino.h>
#include <map>
#include "config/all.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#endif

class Module {
 private:
  String _fileName;
  String _name;
  String _type;

 protected:
  virtual String serialize() = 0;
  virtual void deserialize(const JsonObject& configJson) = 0;

 public:
  virtual ~Module(){};
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void loop(){};
  virtual void mqtt(unsigned int type,
                    const char* topic,
                    const char* payload){};

  void initialize(const String& name,
                  const String& fileName,
                  const String& type);
  const String& getName();
  const String& getFile();
  const String& getType();
  String getConfiguration();
  void setConfiguration(const String& config);
  void deleteConfigurationFile();
};

#endif  // MODULE_H