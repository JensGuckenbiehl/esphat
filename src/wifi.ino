#if defined(ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#include <Wifi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif
#include <DNSServer.h>

DNSServer _dnsServer;
MDNSResponder _mdns;

unsigned int wifi_state = WIFI_START;

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

void _wifiInitCommands() {
  Terminal::addCommand(
      F("WIFI.STATUS"),
      [](int argc, char** argv, Print* response) { logWifiStatus(response); });
  Terminal::addCommand(F("WIFI.RECONNECT"),
                       [](int argc, char** argv, Print* response) {
                         reconnectWifi();
                         response->println(F("Reconnect Wifi"));
                       });
}

// -----------------------------------------------------------------------------
// Public
// -----------------------------------------------------------------------------

unsigned int getWifiState() {
  return wifi_state;
}

bool wifiConnected() {
  return wifi_state == WIFI_CONNECTED_STA;
}

void reconnectWifi() {
  WiFi.disconnect(true);
  wifi_state = WIFI_START;
}

String getIP() {
  if (WiFi.getMode() == WIFI_AP) {
    return WiFi.softAPIP().toString();
  }
  return WiFi.localIP().toString();
}

void logWifiStatus(Print* response) {
  if (WiFi.getMode() == WIFI_AP_STA) {
    response->println(
        F("[WIFI] MODE AP + STA --------------------------------"));
  } else if (WiFi.getMode() == WIFI_AP) {
    response->println(
        F("[WIFI] MODE AP --------------------------------------"));
  } else if (WiFi.getMode() == WIFI_STA) {
    response->println(
        F("[WIFI] MODE STA -------------------------------------"));
  } else {
    response->println(
        F("[WIFI] MODE OFF -------------------------------------"));
    response->println(F("[WIFI] No connection"));
  }
  response->print(F("[WIFI] wifi_state "));
  response->println(wifi_state);
  if (WiFi.status() != WL_CONNECTED) {
    response->println(F("[WIFI] Disconnected from wifi"));
    return;
  }
  if ((WiFi.getMode() & WIFI_AP) == WIFI_AP) {
    response->print(F("[WIFI] IP\t"));
    response->println(WiFi.softAPIP().toString());
    response->print(F("[WIFI] MAC\t"));
    response->println(WiFi.softAPmacAddress());
  }

  if ((WiFi.getMode() & WIFI_STA) == WIFI_STA) {
    uint8_t* bssid = WiFi.BSSID();
    response->print(F("[WIFI] SSID\t"));
    response->println(WiFi.SSID());
    response->print(F("[WIFI] IP\t"));
    response->println(WiFi.localIP().toString());
    response->print(F("[WIFI] MAC\t"));
    response->println(WiFi.macAddress());
    response->print(F("[WIFI] GW\t"));
    response->println(WiFi.gatewayIP().toString());
    response->print(F("[WIFI] DNS\t"));
    response->println(WiFi.dnsIP().toString());
    response->print(F("[WIFI] MASK\t"));
    response->println(WiFi.subnetMask().toString());
#if defined(ARDUINO_ARCH_ESP32)
    response->print(F("[WIFI] HOST\t"));
    response->println(WiFi.getHostname());
#elif defined(ARDUINO_ARCH_ESP8266)
    response->print(F("[WIFI] HOST\t"));
    response->println(WiFi.hostname());
#endif
    response->print(F("[WIFI] BSSID\t"));
    response->print(bssid[0], HEX);
    response->print(bssid[1], HEX);
    response->print(bssid[2], HEX);
    response->print(bssid[3], HEX);
    response->print(bssid[4], HEX);
    response->print(bssid[5], HEX);
    response->println(bssid[6], HEX);
    response->print(F("[WIFI] CH\t"));
    response->println(WiFi.channel());
    response->print(F("[WIFI] RSSI\t"));
    response->println(WiFi.RSSI());
  }

  response->println(F("[WIFI] ----------------------------------------------"));
}

// -----------------------------------------------------------------------------
// Init and loop
// -----------------------------------------------------------------------------

void wifiSetup() {
  WiFi.disconnect(true);
  WiFi.setAutoReconnect(true);
  _wifiInitCommands();
  registerLoop(wifiLoop);
}

void wifiLoop() {
  static unsigned int timeout;

  if (wifi_state == WIFI_START) {
    String mode = getSettings()->get("wifi_mode");
    if (mode == "ap")
      wifi_state = WIFI_START_AP;
    else if (mode == "sta")
      wifi_state = WIFI_START_STA;
    else
      wifi_state = WIFI_START_AUTO;
    getVariables()->set("wifiState", String(wifi_state));
    return;
  }
  if (wifi_state == WIFI_START_AP) {
    String ap_ssid = getSettings()->get("wifi_ap_ssid",
                                        String() + DEVICE + "_" + getChipId());
    String ap_passphrase = getSettings()->get("wifi_ap_pass");
    Terminal::log(F("Started WiFi Acces Point with SSID %s and Passphrase %s"),
                  ap_ssid.c_str(), ap_passphrase.c_str());

    WiFi.mode(WIFI_AP);
    if (ap_passphrase.length()) {
      WiFi.softAP(ap_ssid.c_str(), ap_passphrase.c_str());
    } else {
      WiFi.softAP(ap_ssid.c_str());
    }
    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(53, "*", WiFi.softAPIP());
    Terminal::log(F("Started AP. IP is %s"),
                  WiFi.softAPIP().toString().c_str());
    ++wifi_state;
    getVariables()->set("wifiState", String(wifi_state));
    return;
  }
  if (wifi_state == WIFI_CONNECTED_AP) {
    _dnsServer.processNextRequest();
    return;
  }
  if (wifi_state == WIFI_START_STA || wifi_state == WIFI_START_AUTO) {
    String ssid = getSettings()->get("wifi_ssid", "", true);
    String passphrase = getSettings()->get("wifi_pass", "", true);
    if (!ssid.length() && wifi_state == WIFI_START_AUTO) {
      wifi_state = WIFI_START_AP;
      getVariables()->set("wifiState", String(wifi_state));
      return;
    }
    WiFi.mode(WIFI_STA);
    String hostname = getSettings()->get("hostname", "", true);
    if (hostname.length()) {
#if defined(ARDUINO_ARCH_ESP32)
      WiFi.setHostname(hostname.c_str());
#elif defined(ARDUINO_ARCH_ESP8266)
      WiFi.hostname(hostname);
#endif
    }
    WiFi.begin(ssid.c_str(), passphrase.c_str());
    if (WiFi.status() == WL_IDLE_STATUS) {
      WiFi.disconnect();
    } else {
      Terminal::log(F("Connecting to %s"), ssid.c_str());
      timeout = millis() + WIFI_TIMEOUT;
      ++wifi_state;
      getVariables()->set("wifiState", String(wifi_state));
    }
    return;
  }
  if (wifi_state == WIFI_CONNECT_AUTO) {
    if (WiFi.status() == WL_CONNECTED) {
      wifi_state = WIFI_CONNECT_STA;
      getVariables()->set("wifiState", String(wifi_state));
    } else if (millis() > timeout) {
      Terminal::logLevel(Loglevel::WARNING, F("Can't connect to Wi-Fi!"));
      wifi_state = WIFI_START_AP;
      getVariables()->set("wifiState", String(wifi_state));
    }
    return;
  }
  if (wifi_state == WIFI_CONNECT_STA) {
    if (WiFi.status() == WL_CONNECTED) {
      Terminal::log(F("Connected. IP: %s"), WiFi.localIP().toString().c_str());
      String hostname = getSettings()->get("hostname");
      if (hostname.length()) {
        if (_mdns.begin(hostname.c_str())) {
          Terminal::log(F("MDNS hostname: %s"), hostname.c_str());
        }
      }
      ++wifi_state;
      getVariables()->set("wifiState", String(wifi_state));
      return;
    } else if (millis() > timeout) {
      Terminal::logLevel(Loglevel::WARNING,
                         F("Can't connect to Wi-Fi, try again"));
      wifi_state = WIFI_START_STA;
      getVariables()->set("wifiState", String(wifi_state));
    }
    return;
  }
  if (wifi_state == WIFI_CONNECTED_STA) {
    if (WiFi.status() != WL_CONNECTED) {
      Terminal::logLevel(Loglevel::WARNING, F("Disconnected from wifi"));
      WiFi.disconnect();
      wifi_state = WIFI_START_STA;
      getVariables()->set("wifiState", String(wifi_state));
      return;
    }
    //_mdns.update();
    return;
  }
  // should never get here
  return;
}
