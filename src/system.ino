#include <Ticker.h>
Ticker _defer_reset;

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

bool _system_send_heartbeat = false;

void _heartbeat() {
  unsigned long uptime_seconds = getUptime();
  unsigned int free_heap = getFreeHeap();

  bool serial = !mqttConnected();

  // -------------------------------------------------------------------------
  // Serial
  // -------------------------------------------------------------------------
  if (serial) {
    Terminal::log(F("[SYSTEM] Uptime: %l seconds"), uptime_seconds);
    Terminal::log(F("[SYSTEM] Free heap: %l bytes"), free_heap);
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
    mqttSend(MQTT_TOPIC_HOSTNAME, getSettings()->get("hostname").c_str());
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

// -----------------------------------------------------------------------------
// Public
// -----------------------------------------------------------------------------

void printFirmewareInfo(Print* output) {
  output->println(F("******************************************"));
  output->printf(F("*** %s\r\n"), APP_NAME);
  output->println(F("******************************************"));
  output->printf(F("Version: %s\r\n"), APP_VERSION);
  output->printf(F("Build at: %s\r\n"), buildTime().c_str());
  output->printf(F("Author: %s\r\n"), APP_AUTHOR);
  output->printf(F("Website: %s\r\n"), APP_WEBSITE);
  output->println(F("******************************************"));
}

void printHardwareInfo(Print* output) {
  output->println(F("***        Hardware Information        ***"));
  output->printf(F("Device: %s\r\n"), DEVICE);
  output->printf(F("Manufacturer: %s\r\n"), MANUFACTURER);
  output->print("\n");
  // -------------------------------------------
  output->printf(F("CPU chip ID: %s\r\n"), getChipId().c_str());
  output->printf(F("CPU frequency: %u MHz\r\n"), ESP.getCpuFreqMHz());
  output->printf(F("SDK version: %s\r\n"), ESP.getSdkVersion());
  output->print("\n");
  // -------------------------------------------
  FlashMode_t mode = ESP.getFlashChipMode();
  output->printf(F("Flash chip ID: 0x%06X\r\n"), ESP.getFlashChipId());
  output->printf(F("Flash speed: %u Hz\r\n"), ESP.getFlashChipSpeed());
  output->printf(F("Flash mode: %s\r\n"),
                 mode == FM_QIO
                     ? "QIO"
                     : mode == FM_QOUT
                           ? "QOUT"
                           : mode == FM_DIO
                                 ? "DIO"
                                 : mode == FM_DOUT ? "DOUT" : "UNKNOWN");
  output->print("\n");
  output->printf(F("Flash sector size: %8u bytes\r\n"), SPI_FLASH_SEC_SIZE);
  output->printf(F("Flash size (CHIP): %8u bytes\r\n"),
                 ESP.getFlashChipRealSize());
  output->printf(F("Flash size (SDK):  %8u bytes / %4d sectors\r\n"),
                 ESP.getFlashChipSize(), sectors(ESP.getFlashChipSize()));
  output->printf(F("Firmware size:     %8u bytes / %4d sectors\r\n"),
                 ESP.getSketchSize(), sectors(ESP.getSketchSize()));
  output->printf(F("OTA size:          %8u bytes / %4d sectors\r\n"),
                 ESP.getFreeSketchSpace(), sectors(ESP.getFreeSketchSpace()));
  output->print("\n");
  // -------------------------------------------------------------------------

  FSInfo fs_info;
  bool fs = SPIFFS.info(fs_info);
  if (fs) {
    output->printf(F("SPIFFS total size: %8u bytes / %4d sectors\r\n"),
                   fs_info.totalBytes, sectors(fs_info.totalBytes));
    output->printf(F("       used size:  %8u bytes\r\n"), fs_info.usedBytes);
    output->printf(F("       block size: %8u bytes\r\n"), fs_info.blockSize);
    output->printf(F("       page size:  %8u bytes\r\n"), fs_info.pageSize);
    output->printf(F("       max files:  %8u\r\n"), fs_info.maxOpenFiles);
    output->printf(F("       max length: %8u\r\n"), fs_info.maxPathLength);
  } else {
    output->println(F("No SPIFFS partition"));
  }
  output->print("\n");
  // -------------------------------------------------------------------------
  output->printf(F("Free heap: %8u bytes\r\n"), getFreeHeap());
  output->println(F("******************************************"));
}

void systemSendHeartbeat() {
  _system_send_heartbeat = true;
}

void reset() {
  ESP.restart();
}

void deferredReset(unsigned long delay) {
  _defer_reset.once_ms(delay, reset);
}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

void _systemInitCommands() {
  Terminal::addCommand(F("HEAP"), [](int argc, char** argv, Print* response) {
    response->printf(F("Free heap: %8u bytes\r\n"), getFreeHeap());
  });
  Terminal::addCommand(F("RESET"), [](int argc, char** argv, Print* response) {
    ESP.restart();
  });
  Terminal::addCommand(F("FORMAT"), [](int argc, char** argv, Print* response) {
    SPIFFS.format();
    ESP.restart();
  });
  Terminal::addCommand(F("UPTIME"), [](int argc, char** argv, Print* response) {
    response->printf(F("Uptime: %8u seconds\r\n"), getUptime());
  });
  Terminal::addCommand(F("GPIO"), [](int argc, char** argv, Print* response) {
    if (argc < 2) {
      response->println(F("-ERROR: Wrong arguments"));
      return;
    }
    int pin = String(argv[1]).toInt();
    // if (!gpioValid(pin)) {
    //    DEBUG_MSG_P(PSTR("-ERROR: Invalid GPIO\n"));
    //    return;
    //}
    if (argc > 2) {
      bool state = String(argv[2]).toInt() == 1;
      digitalWrite(pin, state);
    }
    response->printf(F("GPIO %d is %s\r\n"), pin,
                     digitalRead(pin) == HIGH ? "HIGH" : "LOW");
  });
  Terminal::addCommand(F("HARDWAREINFO"),
                       [](int argc, char** argv, Print* response) {
                         printHardwareInfo(response);
                       });
  Terminal::addCommand(F("FIRMEWAREINFO"),
                       [](int argc, char** argv, Print* response) {
                         printFirmewareInfo(response);
                       });
}

// -----------------------------------------------------------------------------
// Init and loop
// -----------------------------------------------------------------------------

void systemSetup() {
  _systemInitCommands();
  registerLoop(systemLoop);
  Terminal::executeCommand("FIRMEWAREINFO");
  Terminal::executeCommand("HARDWAREINFO");
}

void systemLoop() {
  // Heartbeat
  static unsigned long last = 0;
  if (_system_send_heartbeat || (last == 0) ||
      (millis() - last > HEARTBEAT_INTERVAL)) {
    _system_send_heartbeat = false;
    last = millis();
    _heartbeat();
  }
}
