#include "health.h"
#include <Preferences.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <rtcClock/rtcClock.h>
#include <settings/settings.h>
#include <telegram/telegram.h>

// The Arduino core marks a new firmware valid as soon as it boots, which makes rollback useless.
// Returning true here keeps a freshly updated image "pending" until healthTask confirms it after it has
// run for CONFIRM_AFTER_MS. If it crashes or loses power before that, the bootloader goes back to the
// previous firmware. (A serial upload is never pending, so this only affects over-the-air updates.)
extern "C" bool verifyRollbackLater() { return true; }

namespace
{
  const uint32_t CONFIRM_AFTER_MS = 120000;
  const uint32_t CHECK_INTERVAL_MS = 30000;
  const uint32_t LOW_HEAP_BYTES = 30000;  // total free heap
  const uint32_t LOW_BLOCK_BYTES = 20000; // largest free block; TLS needs a big contiguous chunk
  const int LOW_CHECKS_BEFORE_RESTART = 4; // = two minutes in a row
  const int STACK_REPORT_EVERY_CHECKS = 10; // = every five minutes

  // One restart a night keeps a long-running device healthy. Fajr in the summer is after 03:45,
  // so the window is early enough to stay clear of prayer times.
  const uint32_t NIGHTLY_MIN_UPTIME_MS = 20UL * 3600UL * 1000UL;
  const int NIGHTLY_HOUR = 3;
  const int NIGHTLY_FIRST_MINUTE = 10;
  const int NIGHTLY_LAST_MINUTE = 19;

  struct Watched
  {
    const char *name;
    TaskHandle_t handle;
  };
  Watched watched[8];
  int watchedCount = 0;

  const char *resetReasonName(esp_reset_reason_t reason)
  {
    switch (reason)
    {
    case ESP_RST_POWERON: return "power on";
    case ESP_RST_SW: return "software restart";
    case ESP_RST_PANIC: return "crash (panic)";
    case ESP_RST_INT_WDT: return "interrupt watchdog";
    case ESP_RST_TASK_WDT: return "task watchdog";
    case ESP_RST_WDT: return "other watchdog";
    case ESP_RST_BROWNOUT: return "brownout (weak power)";
    case ESP_RST_DEEPSLEEP: return "deep sleep";
    default: return "unknown";
    }
  }

  void restartBecause(const char *why)
  {
    Preferences prefs;
    prefs.begin("health", false);
    prefs.putString("lastRestart", why);
    prefs.end();
    Serial.printf("Restarting: %s\n", why);
    delay(200);
    esp_restart();
  }

  bool nightlyRestartDue()
  {
    if (millis() < NIGHTLY_MIN_UPTIME_MS || isOTAUpdating)
      return false;
    const DateTime now = rtcNow();
    if (now.hour() != NIGHTLY_HOUR || now.minute() < NIGHTLY_FIRST_MINUTE || now.minute() > NIGHTLY_LAST_MINUTE)
      return false;
    const Settings cfg = settings::get();
    return !(cfg.alarmEnabled && cfg.alarmHour == NIGHTLY_HOUR); // never next to an alarm
  }
}

void healthInit()
{
  Preferences prefs;
  prefs.begin("health", false);
  const String planned = prefs.getString("lastRestart", "none");
  prefs.putString("lastRestart", "none");
  prefs.end();
  Serial.printf("Boot: reset reason = %s, planned restart = %s\n", resetReasonName(esp_reset_reason()), planned.c_str());
}

void healthWatchTask(const char *name, TaskHandle_t handle)
{
  if (handle && watchedCount < 8)
    watched[watchedCount++] = {name, handle};
}

void healthTask(void *parameter)
{
  int lowChecks = 0;
  int checks = 0;
  bool confirmed = false;
  for (;;)
  {
    if (!confirmed && millis() > CONFIRM_AFTER_MS)
    {
      confirmed = true;
      const esp_err_t result = esp_ota_mark_app_valid_cancel_rollback();
      Serial.printf("Health: firmware confirmed (%s)\n", result == ESP_OK ? "ok" : esp_err_to_name(result));
    }

    const uint32_t freeHeap = ESP.getFreeHeap();
    const uint32_t largestBlock = ESP.getMaxAllocHeap();
    Serial.printf("Health: heap free=%u largest block=%u lowest ever=%u\n", freeHeap, largestBlock, ESP.getMinFreeHeap());

    if (++checks % STACK_REPORT_EVERY_CHECKS == 0)
    {
      for (int i = 0; i < watchedCount; i++)
        Serial.printf("Health: stack never below %u bytes free in %s\n", (unsigned)uxTaskGetStackHighWaterMark(watched[i].handle), watched[i].name);
    }

    lowChecks = (freeHeap < LOW_HEAP_BYTES || largestBlock < LOW_BLOCK_BYTES) ? lowChecks + 1 : 0;
    if (lowChecks >= LOW_CHECKS_BEFORE_RESTART)
      restartBecause("low memory");
    if (nightlyRestartDue())
      restartBecause("nightly restart");

    vTaskDelay(CHECK_INTERVAL_MS / portTICK_PERIOD_MS);
  }
}
