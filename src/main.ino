#include <EEPROM.h>
#include <vector>

#include "config/all.h"

// -----------------------------------------------------------------------------
// BOOTING
// -----------------------------------------------------------------------------
void setup() {
  terminalSetup();
  systemSetup();
  settingsSetup();

  wifiSetup();
  otaSetup();
  mqttSetup();

  telnetSetup();
#ifdef OUTPUT_SUPPORT
  // outputSetup();
#endif
  modulesSetup();
}

// -----------------------------------------------------------------------------
// LOOPING
// -----------------------------------------------------------------------------

std::vector<loop_callback_f> _loop_callbacks;

void registerLoop(loop_callback_f callback) {
  _loop_callbacks.push_back(callback);
}

unsigned int lastRequest = 0;

void loop() {
  // Call registered loop callbacks
  for (unsigned char i = 0; i < _loop_callbacks.size(); i++) {
    (_loop_callbacks[i])();
  }
}