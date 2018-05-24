#include "keyvaluestore.h"

KeyValueStore settings;
KeyValueStore variables;

void settingsSetup() {
  settings.init("/settings.json");

  Terminal::addCommand(
      F("SETTINGS"), [](int argc, char** argv, Print* response) {
        if (argc < 2) {
          settings.print(response);
          return;
        }
        String key = String(argv[1]);
        if (argc > 2) {
          String value = String(argv[2]);
          settings.set(key, value);
        }
        response->printf(F("Setting %s\t=\t%s\r\n"), key.c_str(),
                         settings.get(key).c_str());
      });
  Terminal::addCommand(F("VARS"), [](int argc, char** argv, Print* response) {
    if (argc < 2) {
      variables.print(response);
      return;
    }
    String key = String(argv[1]);
    response->printf(F("Variable %s\t=\t%s\r\n"), key.c_str(),
                     variables.get(key).c_str());
  });
}

KeyValueStore* getSettings() {
  return &settings;
}

KeyValueStore* getVariables() {
  return &variables;
}