#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <mbedtls/sha256.h>
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

  String toHex(const uint8_t *bytes, size_t length)
  {
    static const char digits[] = "0123456789abcdef";
    String hex;
    hex.reserve(length * 2);
    for (size_t i = 0; i < length; i++)
    {
      hex += digits[bytes[i] >> 4];
      hex += digits[bytes[i] & 0x0F];
    }
    return hex;
  }

  // Copies `total` bytes of firmware from the download into the update partition, hashing them on the
  // way. Returns false if the download stalls or flash writing fails.
  bool streamToFlash(Stream &stream, size_t total, String &sha256Hex)
  {
    mbedtls_sha256_context hash;
    mbedtls_sha256_init(&hash);
    mbedtls_sha256_starts_ret(&hash, 0);

    uint8_t buffer[1024];
    size_t done = 0;
    unsigned long lastData = millis();
    bool ok = true;
    while (done < total)
    {
      const size_t waiting = stream.available();
      if (waiting == 0)
      {
        if (millis() - lastData > 15000)
        {
          ok = false; // the download stalled
          break;
        }
        delay(1);
        continue;
      }
      const size_t want = min(min(waiting, sizeof(buffer)), total - done);
      const size_t got = stream.readBytes(buffer, want);
      if (got == 0)
        continue;
      lastData = millis();
      if (Update.write(buffer, got) != got)
      {
        ok = false;
        break;
      }
      mbedtls_sha256_update_ret(&hash, buffer, got);
      done += got;
    }

    uint8_t digest[32];
    mbedtls_sha256_finish_ret(&hash, digest);
    mbedtls_sha256_free(&hash);
    sha256Hex = toHex(digest, sizeof(digest));
    return ok && done == total;
  }
}

void rememberFirmware(const String &filePath) { firmwarePath = filePath; }

bool hasFirmware() { return firmwarePath.length() > 0; }

void startOtaUpdate(TBMessage &msg, const String &expectedSha256)
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
      if (Update.begin(contentLength))
      {
        Serial.println("Writing firmware to flash...");
        String actualSha256;
        if (!streamToFlash(*http.getStreamPtr(), contentLength, actualSha256))
        {
          Serial.println("Download incomplete or flash write failed");
          Update.abort();
        }
        else if (!actualSha256.equalsIgnoreCase(expectedSha256))
        {
          Serial.println("Checksum mismatch - update refused");
          Serial.println("  expected " + expectedSha256);
          Serial.println("  received " + actualSha256);
          Update.abort();
        }
        else if (Update.end() && Update.isFinished())
        {
          Serial.println("Update complete!");
          otaProgressText.set("DONE!");
          Serial.println("Restarting...");
          delay(2000);
          ESP.restart();
        }
        else
        {
          Serial.print("Update failed: ");
          Serial.println(Update.getError());
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
