#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IotWebConf.h>
#include <Preferences.h>

WiFiClientSecure client;
DNSServer dnsServer;
WebServer server(80);
// Bump when the stored parameter layout changes: the portal then asks for all values again.
#define CONFIG_VERSION "tg2"

// IotWebConf keeps these pointers, so the text is filled in by prepareDeviceIdentity() before init().
static char deviceName[24] = "LedMatrix";
static char setupPassword[12] = "";
static bool rescueStart = false;

IotWebConf iotWebConf(deviceName, &dnsServer, &server, setupPassword, CONFIG_VERSION);

void prepareDeviceIdentity(int rescuePin)
{
  // Button held while powering on: IotWebConf then starts the setup hotspot with the setup password
  // (the way back in if the password chosen during setup is forgotten).
  pinMode(rescuePin, INPUT_PULLUP);
  rescueStart = digitalRead(rescuePin) == LOW;
  iotWebConf.setConfigPin(rescuePin);

  // "LedMatrix-A1B2": the last two bytes of the chip's MAC address, to tell devices apart
  const uint64_t mac = ESP.getEfuseMac();
  snprintf(deviceName, sizeof(deviceName), "LedMatrix-%02X%02X", (unsigned)((mac >> 40) & 0xFF), (unsigned)((mac >> 32) & 0xFF));

  Preferences prefs;
  prefs.begin("wifi", false);
  String saved = prefs.getString("setupPw", "");
  if (saved.length() != 8)
  {
    // No 0/O/1/I/L: the password is read off the matrix and typed on a phone
    static const char alphabet[] = "23456789ABCDEFGHJKMNPQRSTUVWXYZ";
    saved = "";
    for (int i = 0; i < 8; i++)
      saved += alphabet[(esp_random() ^ micros()) % (sizeof(alphabet) - 1)];
    prefs.putString("setupPw", saved);
  }
  prefs.end();
  strlcpy(setupPassword, saved.c_str(), sizeof(setupPassword));
  Serial.printf("First-time setup WiFi: %s  password: %s\n", deviceName, setupPassword);
}

const char *setupWifiName() { return deviceName; }
const char *setupWifiPassword() { return setupPassword; }
bool waitingForFirstSetup()
{
  const iotwebconf::NetworkState state = iotWebConf.getState();
  if (state == iotwebconf::OnLine)
    rescueStart = false;
  return state == iotwebconf::NotConfigured || rescueStart;
}

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
