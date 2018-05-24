#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <ArduinoJson.h>
#include <keyvaluestore.h>
#include <rtstream.h>
#include <terminal.h>
#include <functional>

#if defined(ARDUINO_ARCH_ESP32)
#include <AsyncTCP.h>
#include <SPIFFS.h>
#endif
#include <FS.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESPAsyncTCP.h>
#include "spi_flash.h"
#endif

// -----------------------------------------------------------------------------
// Settings & Variables
// -----------------------------------------------------------------------------
KeyValueStore* getSettings();
KeyValueStore* getVariables();

// -----------------------------------------------------------------------------
// Loop
// -----------------------------------------------------------------------------
typedef std::function<void()> loop_callback_f;
void registerLoop(loop_callback_f callback);

// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------
typedef enum {
  WIFI_START = 0,
  WIFI_START_AP = 1,
  WIFI_CONNECTED_AP = 2,
  WIFI_START_STA = 3,
  WIFI_CONNECT_STA = 4,
  WIFI_CONNECTED_STA = 5,
  WIFI_START_AUTO = 6,
  WIFI_CONNECT_AUTO = 7
} wifi_status_t;
bool wifiConnected();
unsigned int getWifiState();

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------
typedef std::function<void(unsigned int, const char*, const char*)>
    mqtt_callback_f;
void mqttRegister(mqtt_callback_f callback);
void mqttSend(const char* topic, const char* message);
void mqttSendRaw(const char* topic, const char* message, bool retain);
void mqttUnsubscribe(const char* topic);
void mqttUnsubscribeRaw(const char* topic);
void mqttSubscribe(const char* topic);
void mqttSubscribeRaw(const char* topic);
bool mqttConnected();

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
#ifdef OUTPUT_SUPPORT
bool getOutput(int id);
void setOutput(int id, bool value);
bool manualModeOutput();
#endif

#endif  // PROTOTYPES_H