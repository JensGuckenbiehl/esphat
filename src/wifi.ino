#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#include <ESPmDNS.h>
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
    settingsRegisterCommand(F("WIFI.STATUS"), [](Embedis* e) {
        logWifiStatus();
    });
    settingsRegisterCommand(F("WIFI.RECONNECT"), [](Embedis* e) {
        reconnectWifi();
        Log.notice(F("Reconnect Wifi" CR));
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

void logWifiStatus() {

    if (WiFi.getMode() == WIFI_AP_STA) {
        Log.notice(F("[WIFI] MODE AP + STA --------------------------------\n"));
    } else if (WiFi.getMode() == WIFI_AP) {
        Log.notice(F("[WIFI] MODE AP --------------------------------------\n"));
    } else if (WiFi.getMode() == WIFI_STA) {
        Log.notice(F("[WIFI] MODE STA -------------------------------------\n"));
    } else {
        Log.notice(F("[WIFI] MODE OFF -------------------------------------\n"));
        Log.notice(F("[WIFI] No connection\n"));
    }
    Log.notice(F("[WIFI] wifi_state %d\n"), wifi_state);
    if (WiFi.status() != WL_CONNECTED) {
        Log.notice(F("[WIFI] Disconnected from wifi" CR));
        return;
    }
    if ((WiFi.getMode() & WIFI_AP) == WIFI_AP) {
        Log.notice(F("[WIFI] IP    %s\n"), WiFi.softAPIP().toString().c_str());
        Log.notice(F("[WIFI] MAC   %s\n"), WiFi.softAPmacAddress().c_str());
    }

    if ((WiFi.getMode() & WIFI_STA) == WIFI_STA) {
        uint8_t * bssid = WiFi.BSSID();
        Log.notice(F("[WIFI] SSID  %s\n"), WiFi.SSID().c_str());
        Log.notice(F("[WIFI] IP    %s\n"), WiFi.localIP().toString().c_str());
        Log.notice(F("[WIFI] MAC   %s\n"), WiFi.macAddress().c_str());
        Log.notice(F("[WIFI] GW    %s\n"), WiFi.gatewayIP().toString().c_str());
        Log.notice(F("[WIFI] DNS   %s\n"), WiFi.dnsIP().toString().c_str());
        Log.notice(F("[WIFI] MASK  %s\n"), WiFi.subnetMask().toString().c_str());
        #if defined(ARDUINO_ARCH_ESP32)
        Log.notice(F("[WIFI] HOST  %s\n"), WiFi.getHostname());
        #elif defined(ARDUINO_ARCH_ESP8266)
        Log.notice(F("[WIFI] HOST  %s\n"), WiFi.hostname().c_str());
        #endif
        Log.notice(F("[WIFI] BSSID %02X:%02X:%02X:%02X:%02X:%02X\n"),
            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], bssid[6]
        );
        Log.notice(F("[WIFI] CH    %d\n"), WiFi.channel());
        Log.notice(F("[WIFI] RSSI  %d\n"), WiFi.RSSI());
    }

    Log.notice(F("[WIFI] ----------------------------------------------\n"));

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
        String mode = getSetting("wifi_mode");
        if (mode == "ap") wifi_state = WIFI_START_AP;
        else if (mode == "sta") wifi_state = WIFI_START_STA;
        else wifi_state = 6;
        return;
    }
    if (wifi_state == WIFI_START_AP) {
        String ap_ssid = getSetting("wifi_ap_ssid", String() + DEVICE + "_" + getChipId());
        String ap_passphrase = getSetting("wifi_ap_pass");
        Log.notice(F("Started WiFi Acces Point with SSID %s and Passphrase %s" CR), ap_ssid.c_str(), ap_passphrase.c_str());   

        WiFi.mode(WIFI_AP);
        if(ap_passphrase.length()) {
            WiFi.softAP(ap_ssid.c_str(), ap_passphrase.c_str());
        } else {
            WiFi.softAP(ap_ssid.c_str());
        }
        _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        _dnsServer.start(53, "*", WiFi.softAPIP());
        Log.notice(F("Started AP. IP is %s" CR), WiFi.softAPIP().toString().c_str());
        ++wifi_state;
        return;
    }
    if (wifi_state == WIFI_CONNECTED_AP) {
        _dnsServer.processNextRequest();
        return;
    }
    if (wifi_state == WIFI_START_STA || wifi_state == WIFI_START_AUTO) {
        String ssid = getSetting("wifi_ssid");
        String passphrase = getSetting("wifi_pass");
        if (!ssid.length() && wifi_state == WIFI_START_AUTO) {
            wifi_state = WIFI_START_AP; 
            return;
        }
        WiFi.mode(WIFI_STA);
        String hostname = getSetting("hostname");
        if(hostname.length()) {
            #if defined(ARDUINO_ARCH_ESP32)
            WiFi.setHostname(hostname.c_str());
            #elif defined(ARDUINO_ARCH_ESP8266)
            WiFi.hostname(hostname);
            #endif
        }
        WiFi.begin ( ssid.c_str(), passphrase.c_str() );
        if (WiFi.status() == WL_IDLE_STATUS) {
            WiFi.disconnect();
        } else {
            Log.notice(F("Connecting to %s" CR), ssid.c_str());
            timeout = millis() + WIFI_TIMEOUT;
            ++wifi_state;
        }
        return;
    }
    if (wifi_state == WIFI_CONNECT_AUTO) {
        if (WiFi.status() == WL_CONNECTED) {
            wifi_state = WIFI_CONNECT_STA;
        }
        else if (millis() > timeout) {
            Log.warning(F("Can't connect to Wi-Fi!" CR));
            wifi_state = WIFI_START_AP;
        }
        return;
    }
    if (wifi_state == WIFI_CONNECT_STA) {
        if (WiFi.status() == WL_CONNECTED) {
            Log.notice(F("Connected. IP: %s" CR), WiFi.localIP().toString().c_str());
            String hostname = getSetting("hostname");
            if (hostname.length()) {
                if(_mdns.begin ( hostname.c_str()) ) {
                    Log.notice(F("MDNS hostname: %s" CR), hostname.c_str());
                }
            }
            ++wifi_state;
            return;
        } else if (millis() > timeout) {
            Log.warning(F("Can't connect to Wi-Fi, try again" CR));
            wifi_state = WIFI_START_STA;
        }
        return;
    }
    if (wifi_state == WIFI_CONNECTED_STA) {
        if (WiFi.status() != WL_CONNECTED) {
            Log.warning(F("Disconnected from wifi" CR));
            WiFi.disconnect();
            wifi_state = WIFI_START_STA;
            return;
        }
        //_mdns.update();
        return;
    }
    // should never get here
    return;
}
