#ifndef _WIFI_H    // Ensure the header guard name is unique (usually based on the file name)
#define _WIFI_H

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IotWebConf.h>

extern WiFiClientSecure client;
extern DNSServer dnsServer;
extern WebServer server;
extern IotWebConf iotWebConf;

void handleRoot();

// This device's own WiFi name and first-time setup password. The password is random, made once
// and kept in flash, so there is no default that every device shares. Call first thing in setup().
void prepareDeviceIdentity(int rescuePin); // holding the button on `rescuePin` while powering on forces setup mode
const char *setupWifiName();
const char *setupWifiPassword();
bool waitingForFirstSetup(); // true until WiFi is configured, or after a rescue start, until it is online

// Telegram credentials, entered in the IotWebConf config page (never hardcoded).
void setupTelegramConfig(); // call before iotWebConf.init()
String botToken();
String ownerChatId();
const char* botTokenRaw(); // stable pointer; AsyncTelegram2 keeps it instead of copying

#endif // _WIFI_H
