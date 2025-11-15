#ifndef _WIFI_H    // Ensure the header guard name is unique (usually based on the file name)
#define _WIFI_H

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IotWebConf.h>

#define WIFI_INITIAL_AP_PASSWORD "m12345678"
#define THING_NAME "testThing"

extern WiFiClientSecure client;
extern DNSServer dnsServer;
extern WebServer server;
extern IotWebConf iotWebConf;

void handleRoot();

#endif // _WIFI_H
