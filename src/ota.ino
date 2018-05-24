#include "ArduinoOTA.h"

void otaSetup() {
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname(getSettings()->get("hostname").c_str());
  ArduinoOTA.setPassword(getSettings()->get("adminPass", ADMIN_PASS).c_str());

  ArduinoOTA.onStart([]() {
    Terminal::log(F("[OTA] Start"));
    Terminal::rawLog = true;
  });

  ArduinoOTA.onEnd([]() {
    Terminal::rawLog = false;
    Terminal::log(F("[OTA] Done, restarting..."));
    deferredReset(100);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Terminal::log(F("[OTA] Progress: %d%%\r"), (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Terminal::rawLog = false;
    Terminal::logLevel(Loglevel::ERROR, F("\n[OTA] Error #%d: "), error);
    if (error == OTA_AUTH_ERROR)
      Terminal::logLevel(Loglevel::ERROR, F("Auth Failed\n"));
    else if (error == OTA_BEGIN_ERROR)
      Terminal::logLevel(Loglevel::ERROR, F("Begin Failed\n"));
    else if (error == OTA_CONNECT_ERROR)
      Terminal::logLevel(Loglevel::ERROR, F("Connect Failed\n"));
    else if (error == OTA_RECEIVE_ERROR)
      Terminal::logLevel(Loglevel::ERROR, F("Receive Failed\n"));
    else if (error == OTA_END_ERROR)
      Terminal::logLevel(Loglevel::ERROR, F("End Failed\n"));
  });

  ArduinoOTA.begin();

  registerLoop(otaLoop);
}

void otaLoop() {
  ArduinoOTA.handle();
}