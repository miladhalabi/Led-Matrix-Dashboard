#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_task_wdt.h>
#include <wifi/wifi.h>
#include <telegram/telegram.h>
#include "internal.h"

// OTA update state (read by the display task to show "UPDATING...")
volatile bool isOTAUpdating = false;
SharedString otaProgressText;

namespace
{
  String firmwarePath; // Telegram file path of the last .bin the owner sent
}

void rememberFirmware(const String &filePath) { firmwarePath = filePath; }

bool hasFirmware() { return firmwarePath.length() > 0; }

void startOtaUpdate(TBMessage &msg)
{
  bot.sendMessage(msg, "Starting firmware update from Telegram file...");

  // Signal the DisplayTask to show OTA status
  otaProgressText.set("UPDATING...");
  isOTAUpdating = true;

  // AsyncTelegram2 may return a full URL or just a relative path
  String url;
  if (firmwarePath.startsWith("https://"))
  {
    url = firmwarePath;
  }
  else
  {
    url = "https://api.telegram.org/file/bot" + botToken() + "/" + firmwarePath;
  }

  // Disable watchdog to prevent restart during download
  esp_task_wdt_delete(NULL);

  // Close the Telegram connection to free up RAM for the download
  client.stop();

  HTTPClient http;
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  Serial.print("OTA HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode == 200)
  {
    int contentLength = http.getSize();
    Serial.print("OTA File Size: ");
    Serial.println(contentLength);

    if (contentLength > 0)
    {
      WiFiClient *stream = http.getStreamPtr();

      if (Update.begin(contentLength))
      {
        Serial.println("Writing firmware to flash...");
        size_t written = Update.writeStream(*stream);

        if (written == (size_t)contentLength)
        {
          Serial.println("Firmware written successfully!");
          if (Update.end())
          {
            Serial.println("Update complete!");
            if (Update.isFinished())
            {
              otaProgressText.set("DONE!");
              Serial.println("Restarting...");
              delay(2000);
              ESP.restart();
            }
          }
          else
          {
            Serial.print("Update failed: ");
            Serial.println(Update.getError());
          }
        }
        else
        {
          Serial.println("Written bytes mismatch!");
        }
      }
      else
      {
        Serial.println("Not enough space for OTA");
      }
    }
    else
    {
      Serial.println("Content length is 0 or unknown");
    }
  }
  else
  {
    Serial.print("HTTP GET failed: ");
    Serial.println(httpCode);
  }

  http.end();

  // If we reach here, the update failed - reboot to restore Telegram connection
  Serial.println("OTA failed, rebooting to restore connection...");
  delay(1000);
  ESP.restart();
}
