#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IotWebConf.h>

#define WIFI_INITIAL_AP_PASSWORD "m12345678"
#define THING_NAME "testThing"

WiFiClientSecure client;
DNSServer dnsServer;
WebServer server(80);
// Bump when the stored parameter layout changes: the portal then asks for all values again.
#define CONFIG_VERSION "tg1"

IotWebConf iotWebConf(THING_NAME, &dnsServer, &server, WIFI_INITIAL_AP_PASSWORD, CONFIG_VERSION);

static char botTokenValue[64];
static char ownerChatIdValue[24];
static iotwebconf::ParameterGroup telegramGroup = iotwebconf::ParameterGroup("telegram", "Telegram");
// Password parameter: the saved token is never shown again, and leaving the field empty keeps it.
// Telegram tokens are ~46 chars, so IOTWEBCONF_PASSWORD_LEN is raised in platformio.ini.
static iotwebconf::PasswordParameter botTokenParam = iotwebconf::PasswordParameter("Bot token", "botToken", botTokenValue, sizeof(botTokenValue), nullptr, "123456:ABC...");
static iotwebconf::TextParameter ownerChatIdParam = iotwebconf::TextParameter("Owner chat ID", "ownerChatId", ownerChatIdValue, sizeof(ownerChatIdValue), nullptr, "e.g. 123456789");

void setupTelegramConfig()
{
  telegramGroup.addItem(&botTokenParam);
  telegramGroup.addItem(&ownerChatIdParam);
  iotWebConf.addParameterGroup(&telegramGroup);
}

String botToken() { return String(botTokenValue); }
String ownerChatId() { return String(ownerChatIdValue); }
const char* botTokenRaw() { return botTokenValue; }

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
