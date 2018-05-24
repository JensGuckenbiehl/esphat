#ifndef GENERAL_H
#define GENERAL_H

#define ADMIN_PASS "espota"
#define WIFI_TIMEOUT 15000
#define OTA_PORT 8266

//------------------------------------------------------------------------------
// HEARTBEAT
//------------------------------------------------------------------------------
#define HEARTBEAT_INTERVAL \
  300000  // Interval between heartbeat messages (in ms)
#define UPTIME_OVERFLOW 4294967295  // Uptime overflow value

// Topics that will be reported in heartbeat
#define HEARTBEAT_REPORT_STATUS 1
#define HEARTBEAT_REPORT_IP 1
#define HEARTBEAT_REPORT_UPTIME 1
#define HEARTBEAT_REPORT_FREEHEAP 1
#define HEARTBEAT_REPORT_HOSTNAME 1
#define HEARTBEAT_REPORT_APP 1
#define HEARTBEAT_REPORT_VERSION 1

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------
#define MQTT_ENABLED 1  // Enable MQTT connection by default
#define MQTT_SERVER ""  // Default MQTT broker address
#define MQTT_USER ""    // Default MQTT broker usename
#define MQTT_PASS ""    // Default MQTT broker password
#define MQTT_PORT 1883  // MQTT broker port

#define MQTT_TOPIC "{hostname}"  // Default MQTT base topic

#define MQTT_RETAIN true   // MQTT retain flag
#define MQTT_QOS 0         // MQTT QoS value for all messages
#define MQTT_KEEPALIVE 30  // MQTT keepalive value

#define MQTT_RECONNECT_DELAY_MIN \
  5000  // Try to reconnect in 5 seconds upon disconnection
#define MQTT_RECONNECT_DELAY_STEP \
  5000  // Increase the reconnect delay in 5 seconds after each failed attempt
#define MQTT_RECONNECT_DELAY_MAX \
  120000  // Set reconnect time to 2 minutes at most

// These particles will be concatenated to the MQTT_TOPIC base to form the
// actual topic
#define MQTT_TOPIC_ACTION "action"
#define MQTT_TOPIC_IP "ip"
#define MQTT_TOPIC_VERSION "version"
#define MQTT_TOPIC_UPTIME "uptime"
#define MQTT_TOPIC_FREEHEAP "freeheap"
#define MQTT_TOPIC_STATUS "status"
#define MQTT_TOPIC_APP "app"
#define MQTT_TOPIC_HOSTNAME "host"

#define MQTT_STATUS_ONLINE "1"   // Value for the device ON message
#define MQTT_STATUS_OFFLINE "0"  // Value for the device OFF message (will)

#define MQTT_ACTION_RESET "reboot"  // RESET MQTT topic particle

// Internal MQTT events (do not change)
#define MQTT_CONNECT_EVENT 0
#define MQTT_DISCONNECT_EVENT 1
#define MQTT_MESSAGE_EVENT 2

// -----------------------------------------------------------------------------
// MODULES
// -----------------------------------------------------------------------------

#define MODULES_FILE "/modules.json"
#define MOUDULES_MQTT_TOPIC "modules"
#define MOUDULES_ADD_MQTT_TOPIC "modules/add"
#define MOUDULES_GET_MQTT_TOPIC "modules/get"

#define MOUDULE_MQTT_TOPIC "module/+"
#define MOUDULE_GET_MQTT_TOPIC "module/+/get"
#define MOUDULE_SET_MQTT_TOPIC "module/+/set"
#define MOUDULE_DELETE_MQTT_TOPIC "module/+/delete"

//------------------------------------------------------------------------------
// EEPROM
//------------------------------------------------------------------------------

#define EEPROM_SIZE 4096        // EEPROM size in bytes
#define EEPROM_OUTPUT_STATUS 0  // Address for the relay status (1 byte)

//------------------------------------------------------------------------------
// TERMINAL
//------------------------------------------------------------------------------
#define TERMINAL_BUFFER_SIZE 128  // Max size for commands commands

//------------------------------------------------------------------------------
// TELNET
//------------------------------------------------------------------------------
#define TELNET_PORT 23        // Port to listen to telnet clients
#define TELNET_MAX_CLIENTS 1  // Max number of concurrent telnet clients

#endif  // GENERAL_H