#include "outputpin.h"

OutputPin::OutputPin(uint8_t pin, bool initialState, bool inverted)
    : _pin(pin),
      _initialState(initialState),
      _inverted(inverted),
      _enabled(false),
      _currentState(initialState),
      _onTime(0),
      _offTime(0),
      _lastUpdate(0) {
  pinMode(_pin, OUTPUT);
  setOutput(_initialState);
}

OutputPin::~OutputPin() {
  setOutput(_initialState);
}

void OutputPin::setOutput(bool state) {
  digitalWrite(_pin, _inverted ? !state : state);
  _currentState = state;
}

void OutputPin::update() {
  if (!_enabled)
    return;

  if (_currentState && (millis() - _lastUpdate >= _onTime)) {
    setOutput(!_currentState);
    _lastUpdate = millis();
  } else if (!_currentState && (millis() - _lastUpdate >= _offTime)) {
    setOutput(!_currentState);
    _lastUpdate = millis();
  }
}

void OutputPin::on() {
  _enabled = false;
  setOutput(true);
}

void OutputPin::off() {
  _enabled = false;
  setOutput(false);
}

void OutputPin::blink(unsigned int onTime, unsigned int offTime) {
  if (!_enabled) {
    _enabled = true;
    _lastUpdate = millis();
  }
  _onTime = onTime;
  _offTime = offTime;
}