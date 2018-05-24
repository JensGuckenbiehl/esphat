#ifndef LEDMODULE_H
#define LEDMODULE_H

#include "module.h"

#include <outputpin.h>

class LedModule : public Module, public KeyValueStoreNotifier {
 private:
  int _gpio;
  String _mode;
  bool _inverted;
  OutputPin* _led;

  void update();

 public:
  LedModule();
  ~LedModule();
  void start();
  void stop();
  void loop();
  void handleKeyValueStoreChange(const String& key,
                                 const String& value,
                                 KeyValueStore* sender);

 protected:
  String serialize();
  void deserialize(const JsonObject& configJson);
};

#endif  // LEDMODULE_H