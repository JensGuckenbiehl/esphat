bool _system_send_heartbeat = false;

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

void _systemInitCommands() {
    settingsRegisterCommand(F("HEAP"), [](Embedis* e) {
        Log.notice(F("Free HEAP: %d bytes" CR), getFreeHeap());
    });
    settingsRegisterCommand(F("RESET"), [](Embedis* e) {
        ESP.restart();
    });
    settingsRegisterCommand(F("FACTORY.RESET"), [](Embedis* e) {
        SPIFFS.format();
        ESP.restart();
    });
    settingsRegisterCommand(F("UPTIME"), [](Embedis* e) {
        Log.notice(F("Uptime: %d seconds" CR), getUptime());
    });
    settingsRegisterCommand(F("INFO"), [](Embedis* e) {
        logFirmewareInfo();
        logHardwareInfo();
    });
     settingsRegisterCommand(F("GPIO"), [](Embedis* e) {
        if (e->argc < 2) {
            Log.notice(F("-ERROR: Wrong arguments" CR));
            return;
        }
        int pin = String(e->argv[1]).toInt();
        //if (!gpioValid(pin)) {
        //    DEBUG_MSG_P(PSTR("-ERROR: Invalid GPIO\n"));
        //    return;
        //}
        if (e->argc > 2) {
            bool state = String(e->argv[2]).toInt() == 1;
            digitalWrite(pin, state);
        }
        Log.notice(F("GPIO %d is %s" CR), pin, digitalRead(pin) == HIGH ? "HIGH" : "LOW");
    });
}

// -----------------------------------------------------------------------------
// Public
// -----------------------------------------------------------------------------

void logFirmewareInfo() {
      Log.notice(F("******************************************" CR));
      Log.notice(F("*** %s" CR), APP_NAME);   
      Log.notice(F("*** Version: %s" CR), APP_VERSION);   
      Log.notice(F("*** Build at: %s" CR), buildTime().c_str());   
      Log.notice(F("*** Author: %s" CR), APP_AUTHOR);  
      Log.notice(F("*** Website: %s" CR), APP_WEBSITE); 
      Log.notice(F("******************************************" CR));   
}

void logHardwareInfo() {
      Log.notice(F("***        Hardware Information        ***" CR));
      Log.notice(F("*** Device: %s" CR), DEVICE);   
      Log.notice(F("*** Manufacturer: %s" CR), MANUFACTURER);   
      Log.notice(F("*** CPU chip id: %s" CR), getChipId().c_str());   
      Log.notice(F("*** CPU frequency: %d MHz" CR), ESP.getCpuFreqMHz());  
      Log.notice(F("*** SDK Version: %s" CR), ESP.getSdkVersion()); 
      Log.notice(F("*** Flash speed: %d Hz" CR), ESP.getFlashChipSpeed()); 
      FlashMode_t mode = ESP.getFlashChipMode();
      Log.notice(F("*** Flash mode: %s" CR), mode == FM_QIO ? "QIO" : mode == FM_QOUT ? "QOUT" : mode == FM_DIO ? "DIO" : mode == FM_DOUT ? "DOUT" : "UNKNOWN");
      Log.notice(F("*** Flash sector size: %d bytes" CR), SPI_FLASH_SEC_SIZE); 
      Log.notice(F("*** Flash size (SDK): %d bytes / %4d sectors" CR),  ESP.getFlashChipSize(), sectors(ESP.getFlashChipSize())); 
      Log.notice(F("*** Free heap: %d bytes" CR), getFreeHeap());
      Log.notice(F("******************************************" CR));   
}

void systemSendHeartbeat() {
    _system_send_heartbeat = true;
}

// -----------------------------------------------------------------------------
// Init and loop
// -----------------------------------------------------------------------------

void systemSetup() {
    EEPROM.begin(EEPROM_SIZE);
    SPIFFS.begin();
    _systemInitCommands();
    registerLoop(systemLoop);
}

void systemLoop() {
     // Heartbeat
    static unsigned long last = 0;
    if (_system_send_heartbeat || (last == 0) || (millis() - last > HEARTBEAT_INTERVAL)) {
        _system_send_heartbeat = false;
        last = millis();
        heartbeat();
    }
}

void heartbeat() {

    unsigned long uptime_seconds = getUptime();
    unsigned int free_heap = getFreeHeap();

    bool serial = !mqttConnected();

    // -------------------------------------------------------------------------
    // Serial
    // -------------------------------------------------------------------------
    if (serial) {
        Log.notice(F("[SYSTEM] Uptime: %l seconds\n"), uptime_seconds);
        Log.notice(F("[SYSTEM] Free heap: %l bytes\n"), free_heap);
    }

    // -------------------------------------------------------------------------
    // MQTT
    // -------------------------------------------------------------------------
    if (!serial) {
        #if (HEARTBEAT_REPORT_APP)
            mqttSend(MQTT_TOPIC_APP, APP_NAME);
        #endif
        #if (HEARTBEAT_REPORT_VERSION)
            mqttSend(MQTT_TOPIC_VERSION, APP_VERSION);
        #endif
        #if (HEARTBEAT_REPORT_HOSTNAME)
            mqttSend(MQTT_TOPIC_HOSTNAME, getSetting("hostname").c_str());
        #endif
        #if (HEARTBEAT_REPORT_IP)
            mqttSend(MQTT_TOPIC_IP, getIP().c_str());
        #endif
        #if (HEARTBEAT_REPORT_UPTIME)
            mqttSend(MQTT_TOPIC_UPTIME, String(uptime_seconds).c_str());
        #endif
        #if (HEARTBEAT_REPORT_FREEHEAP)
            mqttSend(MQTT_TOPIC_FREEHEAP, String(free_heap).c_str());
        #endif
        #if (HEARTBEAT_REPORT_STATUS)
            mqttSend(MQTT_TOPIC_STATUS, MQTT_STATUS_ONLINE);
        #endif
    }
}