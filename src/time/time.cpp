#include <Arduino.h>
#include <esp_task_wdt.h>
#include <time.h>
#include <RTClib.h>
#include <rtcClock/rtcClock.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"
#include <formaters/formaters.h>
#include <utils/utils.h>
#include <display/display.h>
#include <encoder/encoder.h>
#include <telegram/telegram.h>
#include <prayerTime/prayerTime.h>
#include <weather/weather.h>
#include <settings/settings.h>

#define PRESS_PIN 32

DFRobotDFPlayerMini player;

SharedString timeNow;
SharedString dateNow;
bool AlarmExit = false;

void handleTimeTask(void *parameter)
{
  for (;;)
  {
    DateTime now = rtcNow();
    const Settings cfg = settings::get();
    bool isPM;
    String timeString = formatTime12Hour(now.hour(), now.minute(), isPM);
    String displayString = timeString + (isPM ? "." : "");

    String dateStr = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
    String fullDate = dateStr + "  " + weatherTemp.get() + " " + weatherGlyph.get();

    // icons for "alarm is on" (\x01) and "azan is on" (\x02)
    String icons = "";
    if (cfg.alarmEnabled)
      icons += " \x01";
    if (cfg.prayerEnabled)
      icons += " \x02";
    dateNow.set(fullDate + icons);
    timeNow.set(displayString);
    esp_task_wdt_reset();                  // Reset the watchdog timer
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Update every 1 second
  }
}

void HandleAlarmTask()
{
  unsigned long Otime = millis();
  DateTime now = rtcNow();
  const Settings cfg = settings::get();
  if (now.hour() == cfg.alarmHour && now.minute() == cfg.alarmMinute && cfg.alarmEnabled && AlarmExit == false)
  {
    ringSent = true;
    // player.reset();
    // player.volume(cfg.volume);
    myDisplay.print("ALARM");
    while (true)
    {
      unsigned long Ctime = millis();

      if ((Ctime - Otime > 180000) || longpress(PRESS_PIN, 20, 0))
      {
        Serial.println("alarm goes");
        AlarmExit = true;
        break;
      }
      vTaskDelay(20 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }
  }
  else if (now.minute() != cfg.alarmMinute)
  {
    AlarmExit = false;
  }
}

void alarmRing(void *parameter)
{
  unsigned long oldRingTime = 0;
  unsigned long oldAzanTime = 0;
  bool ringed = false;
  bool Azan = false;
  for (;;)
  {
    if (ringSent == true)
    {
      oldRingTime = millis();
      player.loop(settings::get().music);
      ringSent = false;
      ringed = true;
    }
    else if(AzanNow == true)
    {
      Serial.println("AZAN");
      oldAzanTime = millis();
      player.play(5);
      AzanNow = false;
      Azan = true;
    }

    if ((longpress(PRESS_PIN, 20, 0) && ringed == true) || (longpress(PRESS_PIN, 20, 0) && Azan == true)  || ((millis() - oldRingTime > 180000) && ringed == true ) || ((millis() - oldAzanTime > 234000) && Azan == true))
    {
      player.stop();
      if(ringed == true)
      {
        ringed = false;
      }
      else if (Azan == true)
      {
        Azan = false;
      }
    }
  }
}