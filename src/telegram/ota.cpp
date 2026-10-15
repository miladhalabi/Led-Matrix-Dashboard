#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>
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

// The device restarts after every update attempt, so the outcome is kept in flash and the owner is
// told once Telegram is back (reportOtaResult).
static void saveOtaResult(const String &result)
{
  Preferences prefs;
  prefs.begin("health", false);
  prefs.putString("otaResult", result);
  prefs.end();
}

void reportOtaResult()
{
  Preferences prefs;
  prefs.begin("health", false);
  const String result = prefs.getString("otaResult", "");
  if (result.length() > 0)
    prefs.putString("otaResult", "");
  prefs.end();

  const String owner = ownerChatId();
  if (result.length() == 0 || owner.length() == 0)
    return;
  TBMessage target;
  target.chatId = strtoll(owner.c_str(), nullptr, 10); // ids can be larger than 32 bits
  bot.sendMessage(target, result);
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

  String outcome = "Firmware update failed.";

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
          outcome = "Firmware update failed: the download was incomplete.";
          Update.abort();
        }
        else if (!actualSha256.equalsIgnoreCase(expectedSha256))
        {
          Serial.println("Checksum mismatch - update refused");
          Serial.println("  expected " + expectedSha256);
          Serial.println("  received " + actualSha256);
          outcome = "Firmware update refused: the file does not match the SHA-256 you sent. Nothing was installed.";
          Update.abort();
        }
        else if (Update.end() && Update.isFinished())
        {
          Serial.println("Update complete!");
          saveOtaResult("Firmware installed. It stays on trial for 2 minutes and rolls back if it crashes.");
          otaProgressText.set("DONE!");
          Serial.println("Restarting...");
          delay(2000);
          ESP.restart();
        }
        else
        {
          Serial.print("Update failed: ");
          Serial.println(Update.getError());
          outcome = "Firmware update failed: the new image did not pass verification.";
        }
      }
      else
      {
        Serial.println("Not enough space for OTA");
        outcome = "Firmware update failed: the file is too large for the update partition.";
      }
    }
    else
    {
      Serial.println("Content length is 0 or unknown");
      outcome = "Firmware update failed: Telegram did not report the file size.";
    }
  }
  else
  {
    Serial.print("HTTP GET failed: ");
    Serial.println(httpCode);
    outcome = "Firmware update failed: could not download the file (HTTP " + String(httpCode) + ").";
  }

  http.end();

  // If we reach here, the update failed - reboot to restore Telegram connection
  saveOtaResult(outcome);
  Serial.println("OTA failed, rebooting to restore connection...");
  delay(1000);
  ESP.restart();
}
