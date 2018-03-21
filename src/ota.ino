#include "ArduinoOTA.h"

void otaSetup() {
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setHostname(getSetting("hostname").c_str());
    ArduinoOTA.setPassword(getSetting("adminPass", ADMIN_PASS).c_str());

     ArduinoOTA.onStart([]() {
        Log.notice(F("[OTA] Start" CR));
    });

    ArduinoOTA.onEnd([]() {
        Log.notice(F("\n"));
        Log.notice(F("[OTA] Done, restarting..." CR));
        ESP.restart();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Log.notice(F("[OTA] Progress: %d%%\r"), (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Log.error(F("\n[OTA] Error #%d: "), error);
        if (error == OTA_AUTH_ERROR) Log.error(F("Auth Failed\n"));
        else if (error == OTA_BEGIN_ERROR) Log.error(F("Begin Failed\n"));
        else if (error == OTA_CONNECT_ERROR) Log.error(F("Connect Failed\n"));
        else if (error == OTA_RECEIVE_ERROR) Log.error(F("Receive Failed\n"));
        else if (error == OTA_END_ERROR) Log.error(F("End Failed\n"));
    });

    ArduinoOTA.begin();

    registerLoop(otaLoop);
}

void otaLoop() {
    ArduinoOTA.handle();
}