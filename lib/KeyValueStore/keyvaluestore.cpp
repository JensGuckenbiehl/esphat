#include "keyvaluestore.h"

KeyValueStore::KeyValueStore() {
  SPIFFS.begin();
}

void KeyValueStore::init(const String& filename) {
  _filename = filename;
  load(filename);
}

bool KeyValueStore::hasKey(const String& key) const {
  return (_store.find(key) != _store.end());
}

void KeyValueStore::set(String key, String value) {
  if (get(key) != value || !hasKey(key)) {
    _store[key] = value;
    if (_filename.length() > 0) {
      save(_filename);
    }
    for (auto& notifier : _notifiers) {
      notifier->handleKeyValueStoreChange(key, value, this);
    }
  }
}

String KeyValueStore::get(const String& key) {
  return get(key, "");
}

bool KeyValueStore::save(const String& filename) {
  File configFile = SPIFFS.open(filename, "w");
  if (!configFile) {
    return false;
  }

  DynamicJsonBuffer _jsonBuffer;
  JsonObject& _jsonData = _jsonBuffer.createObject();

  for (const auto& x : _store) {
    _jsonData.set(x.first, String{x.second});
  }

  _jsonData.printTo(configFile);

  configFile.close();
  return true;
}

bool KeyValueStore::load(const String& filename) {
  if (!SPIFFS.begin()) {
    return false;
  }

  File configFile = SPIFFS.open(filename, "r");

  if (!configFile) {
    return false;
  }

  DynamicJsonBuffer _jsonBuffer;
  JsonObject& _jsonData = _jsonBuffer.parseObject(configFile);

  if (!_jsonData.success()) {
    return false;
  }

  for (const auto& configItem : _jsonData) {
    set(configItem.key, configItem.value);
  }

  configFile.close();
  return true;
}

void KeyValueStore::regsiterNotifier(KeyValueStoreNotifier* notifier) {
  _notifiers.push_back(notifier);
}

void KeyValueStore::unregsiterNotifier(KeyValueStoreNotifier* notifier) {
  _notifiers.erase(
      std::remove_if(_notifiers.begin(), _notifiers.end(),
                     [notifier](KeyValueStoreNotifier* currnotifier) {
                       return currnotifier == notifier;
                     }),
      _notifiers.end());
}

void KeyValueStore::print(Print* printable) {
  for (const auto& x : _store) {
    printable->print(x.first);
    printable->print("\t=\t");
    printable->println(x.second);
  }
}