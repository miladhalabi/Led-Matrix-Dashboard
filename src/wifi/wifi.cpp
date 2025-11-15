#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IotWebConf.h>

#define WIFI_INITIAL_AP_PASSWORD "m12345678"
#define THING_NAME "testThing"

WiFiClientSecure client;
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(THING_NAME, &dnsServer, &server, WIFI_INITIAL_AP_PASSWORD);

void handleRoot()
{
  
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";
  
  server.send(200, "text/html", s);
}
