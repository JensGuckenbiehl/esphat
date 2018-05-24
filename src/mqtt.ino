#include <AsyncMqttClient.h>

AsyncMqttClient _mqtt;

bool _mqtt_enabled = MQTT_ENABLED;
unsigned long _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
unsigned char _mqtt_qos = MQTT_QOS;
bool _mqtt_retain = MQTT_RETAIN;
unsigned long _mqtt_keepalive = MQTT_KEEPALIVE;
String _mqtt_topic;
char* _mqtt_user = 0;
char* _mqtt_pass = 0;
char* _mqtt_will;
char* _mqtt_clientid;

std::vector<mqtt_callback_f> _mqtt_callbacks;

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void _mqttConnect() {
  // Do not connect if disabled
  if (!_mqtt_enabled)
    return;

  // Do not connect if already connected
  if (_mqtt.connected())
    return;

  // Check reconnect interval
  static unsigned long last = 0;
  if (millis() - last < _mqtt_reconnect_delay)
    return;
  last = millis();

  // Increase the reconnect delay
  _mqtt_reconnect_delay += MQTT_RECONNECT_DELAY_STEP;
  if (_mqtt_reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
  }

  String h = getSettings()->get("mqttServer", MQTT_SERVER);
  char* host = strdup(h.c_str());

  unsigned int port = getSettings()->get("mqttPort", MQTT_PORT).toInt();

  if (_mqtt_user)
    free(_mqtt_user);
  if (_mqtt_pass)
    free(_mqtt_pass);
  if (_mqtt_will)
    free(_mqtt_will);
  if (_mqtt_clientid)
    free(_mqtt_clientid);

  _mqtt_user = strdup(getSettings()->get("mqttUser", MQTT_USER).c_str());
  _mqtt_pass = strdup(getSettings()->get("mqttPassword", MQTT_PASS).c_str());
  _mqtt_will = strdup(mqttTopic(MQTT_TOPIC_STATUS).c_str());
  _mqtt_clientid =
      strdup(getSettings()
                 ->get("mqttClientID",
                       getSettings()->get(
                           "hostname", String() + DEVICE + "_" + getChipId()))
                 .c_str());

  Terminal::log(F("[MQTT] Connecting to broker at %s:%d"), host, port);

  _mqtt.setServer(host, port);
  _mqtt.setClientId(_mqtt_clientid);
  _mqtt.setKeepAlive(_mqtt_keepalive);
  _mqtt.setCleanSession(false);
  _mqtt.setWill(_mqtt_will, _mqtt_qos, _mqtt_retain, "0");
  if ((strlen(_mqtt_user) > 0) && (strlen(_mqtt_pass) > 0)) {
    Terminal::log(F("[MQTT] Connecting as user %s"), _mqtt_user);
    _mqtt.setCredentials(_mqtt_user, _mqtt_pass);
  }

  Terminal::log(F("[MQTT] Client ID: %s"), _mqtt_clientid);
  Terminal::log(F("[MQTT] QoS: %d"), _mqtt_qos);
  Terminal::log(F("[MQTT] Retain flag: %d"), _mqtt_retain ? 1 : 0);
  Terminal::log(F("[MQTT] Keepalive time: %ds"), _mqtt_keepalive);
  Terminal::log(F("[MQTT] Will topic: %s"), _mqtt_will);

  _mqtt.connect();

  free(host);
}

void _mqttConfigure() {
  // Get base topic
  _mqtt_topic = getSettings()->get("mqttTopic", MQTT_TOPIC);
  if (_mqtt_topic.endsWith("/"))
    _mqtt_topic.remove(_mqtt_topic.length() - 1);

  // Placeholders
  _mqtt_topic.replace("{identifier}", String() + "DEVICE_" + getChipId());
  _mqtt_topic.replace("{hostname}", getSettings()->get("hostname"));
  _mqtt_topic.replace("{magnitude}", "#");
  if (_mqtt_topic.indexOf("#") == -1)
    _mqtt_topic = _mqtt_topic + "/#";

  // MQTT options
  _mqtt_qos = getSettings()->get("mqttQoS", MQTT_QOS).toInt();
  _mqtt_retain = getSettings()->get("mqttRetain", MQTT_RETAIN).toInt() == 1;
  _mqtt_keepalive = getSettings()->get("mqttKeep", MQTT_KEEPALIVE).toInt();

  // Enable
  if (getSettings()->get("mqttServer", MQTT_SERVER).length() == 0) {
    _mqtt_enabled = false;
  } else {
    _mqtt_enabled =
        getSettings()->get("mqttEnabled", MQTT_ENABLED).toInt() == 1;
  }

  _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
}

// -----------------------------------------------------------------------------
// MQTT Callbacks
// -----------------------------------------------------------------------------

void _mqttCallback(unsigned int type, const char* topic, const char* payload) {
  if (type == MQTT_CONNECT_EVENT) {
    // Subscribe to internal action topics
    mqttSubscribe(MQTT_TOPIC_ACTION);

    // Flag system to send heartbeat
    systemSendHeartbeat();
  }

  if (type == MQTT_MESSAGE_EVENT) {
    // Match topic
    String t = mqttMagnitude((char*)topic);

    // Actions
    if (t.equals(MQTT_TOPIC_ACTION)) {
      if (strcmp(payload, MQTT_ACTION_RESET) == 0) {
        ESP.restart();
      }
    }
  }
}

void _mqttOnConnect() {
  Terminal::log(F("[MQTT] Connected!"));
  getVariables()->set("mqttState", "1");
  _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

  // Clean subscriptions
  mqttUnsubscribeRaw("#");

  // Send connect event to subscribers
  for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
    (_mqtt_callbacks[i])(MQTT_CONNECT_EVENT, NULL, NULL);
  }
}

void _mqttOnDisconnect() {
  Terminal::logLevel(Loglevel::WARNING, F("[MQTT] Disconnected!"));
  getVariables()->set("mqttState", "0");

  // Send disconnect event to subscribers
  for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
    (_mqtt_callbacks[i])(MQTT_DISCONNECT_EVENT, NULL, NULL);
  }
}

void _mqttOnMessage(char* topic, char* payload, unsigned int len) {
  char message[len + 1];
  if (len == 0) {
    message[0] = '\0';
  } else {
    strlcpy(message, (char*)payload, len + 1);
  }

  Terminal::log(F("[MQTT] Received %s => %s"), topic, message);

  // Send message event to subscribers
  for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
    (_mqtt_callbacks[i])(MQTT_MESSAGE_EVENT, topic, message);
  }
}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

void _mqttInitCommands() {
  Terminal::addCommand(F("MQTT.RECONNECT"),
                       [](int argc, char** argv, Print* response) {
                         mqttReset();
                         response->println(F("Reconnect MQTT"));
                       });

  Terminal::addCommand(F("MQTT.PUBLISH"),
                       [](int argc, char** argv, Print* response) {
                         if (argc < 2) {
                           response->println(F("-ERROR: Wrong arguments"));
                           return;
                         }
                         String topic = String(argv[1]);
                         String payload = "";
                         if (argc > 2) {
                           payload = String(argv[2]);
                         }
                         bool retain = MQTT_RETAIN;
                         if (argc > 3) {
                           retain = String(argv[3]).toInt();
                         }
                         mqttSendRaw(topic.c_str(), payload.c_str(), retain);
                       });
}

// -----------------------------------------------------------------------------
// Public
// -----------------------------------------------------------------------------

bool mqttConnected() {
  return _mqtt.connected();
}

void mqttDisconnect() {
  if (_mqtt.connected()) {
    Terminal::log(F("[MQTT] Disconnecting"));
    _mqtt.disconnect();
  }
}

void mqttRegister(mqtt_callback_f callback) {
  _mqtt_callbacks.push_back(callback);
}

void mqttReset() {
  _mqttConfigure();
  mqttDisconnect();
}

String mqttMagnitude(char* topic) {
  String pattern = _mqtt_topic;
  int position = pattern.indexOf("#");
  if (position == -1)
    return String();
  String start = pattern.substring(0, position);
  String end = pattern.substring(position + 1);

  String magnitude = String(topic);
  if (magnitude.startsWith(start) && magnitude.endsWith(end)) {
    magnitude.replace(start, "");
    magnitude.replace(end, "");
  } else {
    magnitude = String();
  }

  return magnitude;
}

String mqttTopic(const char* magnitude) {
  String output = _mqtt_topic;
  output.replace("#", magnitude);
  return output;
}

void mqttSubscribeRaw(const char* topic) {
  if (_mqtt.connected() && (strlen(topic) > 0)) {
    unsigned int packetId = _mqtt.subscribe(topic, _mqtt_qos);
    Terminal::log(F("[MQTT] Subscribing to %s (PID %d)"), topic, packetId);
  }
}

void mqttSubscribe(const char* topic) {
  mqttSubscribeRaw(mqttTopic(topic).c_str());
}

void mqttUnsubscribeRaw(const char* topic) {
  if (_mqtt.connected() && (strlen(topic) > 0)) {
    unsigned int packetId = _mqtt.unsubscribe(topic);
    Terminal::log(F("[MQTT] Unsubscribing to %s (PID %d)"), topic, packetId);
  }
}

void mqttUnsubscribe(const char* topic) {
  mqttUnsubscribeRaw(mqttTopic(topic).c_str());
}

void mqttSendRaw(const char* topic, const char* message, bool retain) {
  if (_mqtt.connected() && (strlen(topic) > 0)) {
    unsigned int packetId = _mqtt.publish(topic, _mqtt_qos, retain, message);
    Terminal::log(F("[MQTT] Sending %s => %s (PID %d)"), topic, message,
                  packetId);
  }
}

void mqttSend(const char* topic, const char* message) {
  mqttSendRaw(mqttTopic(topic).c_str(), message, _mqtt_retain);
}
// -----------------------------------------------------------------------------
// Init and loop
// -----------------------------------------------------------------------------

void mqttSetup() {
  _mqtt.onConnect([](bool sessionPresent) { _mqttOnConnect(); });
  _mqtt.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
    if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
      Terminal::logLevel(Loglevel::WARNING, F("[MQTT] TCP Disconnected"));
    }
    if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
      Terminal::logLevel(Loglevel::WARNING, F("[MQTT] Identifier Rejected"));
    }
    if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
      Terminal::logLevel(Loglevel::WARNING, F("[MQTT] Server unavailable"));
    }
    if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
      Terminal::logLevel(Loglevel::WARNING, F("[MQTT] Malformed credentials"));
    }
    if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
      Terminal::logLevel(Loglevel::WARNING, F("[MQTT] Not authorized"));
    }
    _mqttOnDisconnect();
  });
  _mqtt.onMessage([](char* topic, char* payload,
                     AsyncMqttClientMessageProperties properties, size_t len,
                     size_t index,
                     size_t total) { _mqttOnMessage(topic, payload, len); });
  _mqtt.onSubscribe([](uint16_t packetId, uint8_t qos) {
    Terminal::log(F("[MQTT] Subscribe ACK for PID %d"), packetId);
  });
  _mqtt.onPublish([](uint16_t packetId) {
    Terminal::log(F("[MQTT] Publish ACK for PID %d"), packetId);
  });

  _mqttConfigure();
  mqttRegister(_mqttCallback);

  _mqttInitCommands();
  registerLoop(mqttLoop);
}

void mqttLoop() {
  if (getWifiState() != WIFI_CONNECTED_STA)
    return;
  _mqttConnect();
}