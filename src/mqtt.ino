#include <AsyncMqttClient.h>

AsyncMqttClient _mqtt;

bool _mqtt_enabled = MQTT_ENABLED;
unsigned long _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
unsigned char _mqtt_qos = MQTT_QOS;
bool _mqtt_retain = MQTT_RETAIN;
unsigned long _mqtt_keepalive = MQTT_KEEPALIVE;
String _mqtt_topic;
char *_mqtt_user = 0;
char *_mqtt_pass = 0;
char *_mqtt_will;
char *_mqtt_clientid;

std::vector<mqtt_callback_f> _mqtt_callbacks;

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void _mqttConnect() {

    // Do not connect if disabled
    if (!_mqtt_enabled) return;

    // Do not connect if already connected
    if (_mqtt.connected()) return;

    // Check reconnect interval
    static unsigned long last = 0;
    if (millis() - last < _mqtt_reconnect_delay) return;
    last = millis();

    // Increase the reconnect delay
    _mqtt_reconnect_delay += MQTT_RECONNECT_DELAY_STEP;
    if (_mqtt_reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
        _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
    }

    String h = getSetting("mqttServer", MQTT_SERVER);
    char * host = strdup(h.c_str());

    unsigned int port = getSetting("mqttPort", MQTT_PORT).toInt();

    if (_mqtt_user) free(_mqtt_user);
    if (_mqtt_pass) free(_mqtt_pass);
    if (_mqtt_will) free(_mqtt_will);
    if (_mqtt_clientid) free(_mqtt_clientid);

    _mqtt_user = strdup(getSetting("mqttUser", MQTT_USER).c_str());
    _mqtt_pass = strdup(getSetting("mqttPassword", MQTT_PASS).c_str());
    _mqtt_will = strdup(mqttTopic(MQTT_TOPIC_STATUS).c_str());
    _mqtt_clientid = strdup(getSetting("mqttClientID", getSetting("hostname", String() + DEVICE + "_" + getChipId())).c_str());

    Log.notice(F("[MQTT] Connecting to broker at %s:%d" CR), host, port);

    _mqtt.setServer(host, port);
    _mqtt.setClientId(_mqtt_clientid);
    _mqtt.setKeepAlive(_mqtt_keepalive);
    _mqtt.setCleanSession(false);
    _mqtt.setWill(_mqtt_will, _mqtt_qos, _mqtt_retain, "0");
    if ((strlen(_mqtt_user) > 0) && (strlen(_mqtt_pass) > 0)) {
        Log.notice(F("[MQTT] Connecting as user %s" CR), _mqtt_user);
        _mqtt.setCredentials(_mqtt_user, _mqtt_pass);
    }

    Log.notice(F("[MQTT] Client ID: %s" CR), _mqtt_clientid);
    Log.notice(F("[MQTT] QoS: %d" CR), _mqtt_qos);
    Log.notice(F("[MQTT] Retain flag: %d" CR), _mqtt_retain ? 1 : 0);
    Log.notice(F("[MQTT] Keepalive time: %ds" CR), _mqtt_keepalive);
    Log.notice(F("[MQTT] Will topic: %s" CR), _mqtt_will);

    _mqtt.connect();

    free(host);

}

void _mqttConfigure() {

    // Get base topic
    _mqtt_topic = getSetting("mqttTopic", MQTT_TOPIC);
    if (_mqtt_topic.endsWith("/")) _mqtt_topic.remove(_mqtt_topic.length()-1);

    // Placeholders
    _mqtt_topic.replace("{identifier}", String() + "DEVICE_" + getChipId());
    _mqtt_topic.replace("{hostname}", getSetting("hostname"));
    _mqtt_topic.replace("{magnitude}", "#");
    if (_mqtt_topic.indexOf("#") == -1) _mqtt_topic = _mqtt_topic + "/#";

    // MQTT options
    _mqtt_qos = getSetting("mqttQoS", MQTT_QOS).toInt();
    _mqtt_retain = getSetting("mqttRetain", MQTT_RETAIN).toInt() == 1;
    _mqtt_keepalive = getSetting("mqttKeep", MQTT_KEEPALIVE).toInt();

    // Enable
    if (getSetting("mqttServer", MQTT_SERVER).length() == 0) {
        _mqtt_enabled = false;
    } else {
        _mqtt_enabled = getSetting("mqttEnabled", MQTT_ENABLED).toInt() == 1;
    }

    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

}

// -----------------------------------------------------------------------------
// MQTT Callbacks
// -----------------------------------------------------------------------------

void _mqttCallback(unsigned int type, const char * topic, const char * payload) {

    if (type == MQTT_CONNECT_EVENT) {

        // Subscribe to internal action topics
        mqttSubscribe(MQTT_TOPIC_ACTION);

        // Flag system to send heartbeat
        systemSendHeartbeat();

    }

    if (type == MQTT_MESSAGE_EVENT) {

        // Match topic
        String t = mqttMagnitude((char *) topic);

        // Actions
        if (t.equals(MQTT_TOPIC_ACTION)) {
            if (strcmp(payload, MQTT_ACTION_RESET) == 0) {
                ESP.restart();
            }
        }

    }

}

void _mqttOnConnect() {

    Log.notice(F("[MQTT] Connected!\n"));
    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

    // Clean subscriptions
    mqttUnsubscribeRaw("#");

    // Send connect event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_CONNECT_EVENT, NULL, NULL);
    }

}

void _mqttOnDisconnect() {

    Log.warning(F("[MQTT] Disconnected!\n"));

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
        strlcpy(message, (char *) payload, len + 1);
    }

    Log.notice(F("[MQTT] Received %s => %s\n"), topic, message);

    // Send message event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_MESSAGE_EVENT, topic, message);
    }

}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

void _mqttInitCommands() {

    settingsRegisterCommand(F("MQTT.RECONNECT"), [](Embedis* e) {
        mqttReset();
        Log.notice(F("Reconnect MQTT" CR));
    });
    settingsRegisterCommand(F("MQTT.PUBLISH"), [](Embedis* e) {
        if (e->argc < 2) {
            Log.error(F("-ERROR: Wrong arguments" CR));
            return;
        }
        String topic = String(e->argv[1]);
        String payload = "";
        if (e->argc > 2) {
            payload = String(e->argv[2]);
        }
        bool retain = MQTT_RETAIN;
        if (e->argc > 2) {
            retain = String(e->argv[2]).toInt();
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
        Log.notice(F("[MQTT] Disconnecting\n"));
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

String mqttMagnitude(char * topic) {

    String pattern = _mqtt_topic;
    int position = pattern.indexOf("#");
    if (position == -1) return String();
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

String mqttTopic(const char * magnitude) {
    String output = _mqtt_topic;
    output.replace("#", magnitude);
    return output;
}

void mqttSubscribeRaw(const char * topic) {
    if (_mqtt.connected() && (strlen(topic) > 0)) {
        unsigned int packetId = _mqtt.subscribe(topic, _mqtt_qos);
        Log.notice(F("[MQTT] Subscribing to %s (PID %d)\n"), topic, packetId);
    }
}

void mqttSubscribe(const char * topic) {
    mqttSubscribeRaw(mqttTopic(topic).c_str());
}

void mqttUnsubscribeRaw(const char * topic) {
    if (_mqtt.connected() && (strlen(topic) > 0)) {
        unsigned int packetId = _mqtt.unsubscribe(topic);
        Log.notice(F("[MQTT] Unsubscribing to %s (PID %d)\n"), topic, packetId);
    }
}

void mqttUnsubscribe(const char * topic) {
    mqttUnsubscribeRaw(mqttTopic(topic).c_str());
}

void mqttSendRaw(const char * topic, const char * message, bool retain) {
    if (_mqtt.connected()) {
        unsigned int packetId = _mqtt.publish(topic, _mqtt_qos, retain, message);
        Log.notice(F("[MQTT] Sending %s => %s (PID %d)" CR), topic, message, packetId);
    }
}

void mqttSend(const char * topic, const char * message) {
        mqttSendRaw(mqttTopic(topic).c_str(), message, _mqtt_retain);
}
// -----------------------------------------------------------------------------
// Init and loop
// -----------------------------------------------------------------------------

void mqttSetup() {
        _mqtt.onConnect([](bool sessionPresent) {
            _mqttOnConnect();
        });
        _mqtt.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
            if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
                Log.warning(F("[MQTT] TCP Disconnected\n"));
            }
            if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
                Log.warning(F("[MQTT] Identifier Rejected\n"));
            }
            if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
                Log.warning(F("[MQTT] Server unavailable\n"));
            }
            if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
                Log.warning(F("[MQTT] Malformed credentials\n"));
            }
            if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
                Log.warning(F("[MQTT] Not authorized\n"));
            }
            _mqttOnDisconnect();
        });
        _mqtt.onMessage([](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
            _mqttOnMessage(topic, payload, len);
        });
        _mqtt.onSubscribe([](uint16_t packetId, uint8_t qos) {
            Log.notice(F("[MQTT] Subscribe ACK for PID %d\n"), packetId);
        });
        _mqtt.onPublish([](uint16_t packetId) {
            Log.notice(F("[MQTT] Publish ACK for PID %d\n"), packetId);
        });


    _mqttConfigure();
    mqttRegister(_mqttCallback);

  
    _mqttInitCommands();
    registerLoop(mqttLoop);

}

void mqttLoop() {
    if (getWifiState() != WIFI_CONNECTED_STA) return;
    _mqttConnect();
}