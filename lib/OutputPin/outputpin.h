#pragma once

#include <Arduino.h>

class OutputPin {
 private:
  uint8_t _pin;
  bool _initialState;
  bool _inverted;
  bool _enabled;
  bool _currentState;

  unsigned int _onTime;
  unsigned int _offTime;
  unsigned long _lastUpdate;

 public:
  OutputPin(uint8_t pin, bool initialState, bool inverted = false);
  ~OutputPin();
  void setOutput(bool state);
  void update();
  void on();
  void off();
  void blink(unsigned int onTime, unsigned int offTime);
};