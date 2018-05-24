#include "buttonmodule.h"

#include "modulefactory.h"
REGISTER_MODULE("button", ButtonModule)

#include "config/all.h"
#include "terminal.h"

ButtonModule::ButtonModule()
    : _gpio(-1),
      _clickCommand(""),
      _doubleClickCommand(""),
      _longClickCommand("") {}

ButtonModule::~ButtonModule() {}

void ButtonModule::start() {
  if (_gpio != -1) {
    pinMode(_gpio, INPUT_PULLUP);
  }
}

void ButtonModule::stop() {
  _gpio = -1;
}

String ButtonModule::serialize() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJson = jsonBuffer.createObject();
  configJson["gpio"] = _gpio;
  configJson["clickCommand"] = _clickCommand;
  configJson["doubleClickCommand"] = _doubleClickCommand;
  configJson["longClickCommand"] = _longClickCommand;
  String output;
  configJson.printTo(output);
  return output;
}

void ButtonModule::deserialize(const JsonObject& configJson) {
  if (configJson.containsKey("gpio")) {
    _gpio = configJson["gpio"];
  }
  if (configJson.containsKey("clickCommand")) {
    _clickCommand = configJson["clickCommand"].as<String>();
  }
  if (configJson.containsKey("doubleClickCommand")) {
    _doubleClickCommand = configJson["doubleClickCommand"].as<String>();
  }
  if (configJson.containsKey("longClickCommand")) {
    _longClickCommand = configJson["longClickCommand"].as<String>();
  }
}

void ButtonModule::loop() {
  if (_gpio == -1) {
    return;
  }

  _multiButton.update(digitalRead(_gpio) == 0);

  String command;

  if (_multiButton.isSingleClick()) {
    command = _clickCommand;
  } else if (_multiButton.isDoubleClick()) {
    command = _doubleClickCommand;
  } else if (_multiButton.isLongClick()) {
    command = _longClickCommand;
  }

  if (command.length() > 0) {
    command = command + "\n";
    Terminal::executeCommand(command.c_str());
  }
}