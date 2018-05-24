#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include <ArduinoJson.h>
#include <arduino.h>
#include <map>
#include <vector>

#if defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#endif
#include <FS.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include "spi_flash.h"
#endif

class KeyValueStore;

class KeyValueStoreNotifier {
 public:
  virtual void handleKeyValueStoreChange(const String& key,
                                         const String& value,
                                         KeyValueStore* sender) = 0;
};

class KeyValueStore {
 private:
  struct cmp_str {
    bool operator()(const String& a, const String& b) const {
      return strcmp(a.c_str(), b.c_str()) < 0;
    }
  };

  std::map<String, String, cmp_str> _store;
  String _filename;
  std::vector<KeyValueStoreNotifier*> _notifiers;

 public:
  explicit KeyValueStore();
  ~KeyValueStore() = default;

  bool hasKey(const String& key) const;
  void set(String key, String value);
  String get(const String& key);
  template <typename T>
  String get(const String& key, T defaultValue, bool saveDefaultValue = false) {
    auto found = _store.find(key);
    if (found != _store.end()) {
      return found->second;
    }
    if (saveDefaultValue) {
      set(key, String(defaultValue));
    }
    return String(defaultValue);
  }

  void print(Print* printable);

  bool save(const String& filename);
  bool load(const String& filename);
  void init(const String& filename);

  void regsiterNotifier(KeyValueStoreNotifier* notifier);
  void unregsiterNotifier(KeyValueStoreNotifier* notifier);
};

#endif  // KEYVALUESTORE_H