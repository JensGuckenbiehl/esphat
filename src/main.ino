#include <ArduinoLog.h>
#include <EEPROM.h>
#include <vector>

#include "config/all.h"

// -----------------------------------------------------------------------------
// BOOTING
// -----------------------------------------------------------------------------

void setup() {
    SERIAL_PORT.begin(115200);

    Log.begin(LOG_LEVEL_VERBOSE, &SERIAL_PORT);
    Log.setPrefix(printTimestamp); 

    logFirmewareInfo();
    logHardwareInfo();

    settingsSetup();
    systemSetup();
    wifiSetup();
    otaSetup();
    mqttSetup();
    telnetSetup();
    #ifdef OUTPUT_SUPPORT
    outputSetup();
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

// -----------------------------------------------------------------------------
// LOGGING HELPER
// -----------------------------------------------------------------------------

void printTimestamp(Print* _logOutput) {
  char c[12];
  int m = sprintf(c, "%10lu ", millis());
  _logOutput->print(c);
}