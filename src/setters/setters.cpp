#include <Arduino.h>
#include <parsers/parsers.h>
#include <converters/converters.h>
#include <time/time.h>
#include <settings/settings.h>

bool alarmSetter(String timeToAlarm)
{
  timeToAlarm.toUpperCase();
  if (timeToAlarm == "ALARM ON")
  {
    settings::setAlarmEnabled(true);
    return true;
  }
  else if (timeToAlarm == "ALARM OFF")
  {
    settings::setAlarmEnabled(false);
    return true;
  }
  else
  {
    String cleanedTimeToAlarm = removeStartWord(timeToAlarm, "ALARM");
    Serial.println(cleanedTimeToAlarm);
    int alarmHourTelegram = 0, alarmMinuteTelegram = 0;
    String NightDay = "";
    String OnOff = "";
    if (parseTime(cleanedTimeToAlarm, alarmHourTelegram, alarmMinuteTelegram, NightDay))
    {
      int hour24Alarm, minute24Alarm;
      convertTo24Hour(alarmHourTelegram, alarmMinuteTelegram, NightDay, hour24Alarm, minute24Alarm);
      settings::setAlarmTime(hour24Alarm, minute24Alarm);
      return true;
    }
    return false;
  }
  return false;
}

bool timeSetter(String timeMessgae)
{
  DateTime getTimeTelegram = rtcNow();
  String cleanedTimeMessage = removeStartWord(timeMessgae, "time");
  int hourTelegram = 0, minuteTelegram = 0;
  int yearTelegram = getTimeTelegram.year();
  int monthTelegram = getTimeTelegram.month();
  int dayTelegram = getTimeTelegram.day();
  String AmPm = "";
  if (parseTime(cleanedTimeMessage, hourTelegram, minuteTelegram, AmPm))
  {
    AmPm.toUpperCase();
    int nowTelegramHour = 0;
    int nowTelegramMinute = 0;
    convertTo24Hour(hourTelegram, minuteTelegram, AmPm, nowTelegramHour, nowTelegramMinute);
    rtcAdjust(DateTime(yearTelegram, monthTelegram, dayTelegram, nowTelegramHour, nowTelegramMinute));
    return true;
  }
  return false;
}

bool dateSetter(String dateMessge)
{
  DateTime getTimeTelegram = rtcNow();
  String cleanedTimeMessage = removeStartWord(dateMessge, "date");
  Serial.println(cleanedTimeMessage);
  int hourTelegram = getTimeTelegram.hour(), minuteTelegram = getTimeTelegram.minute();
  int yearTelegram = 0, monthTelegram = 0, dayTelegram = 0;
  if (parseDate(cleanedTimeMessage, yearTelegram, monthTelegram, dayTelegram))
  {
    rtcAdjust(DateTime(yearTelegram, monthTelegram, dayTelegram, hourTelegram, minuteTelegram));
    return true;
  }
  return false;
}

bool volumeSetter(String volumeMessage)
{
  String cleanedVolume = removeStartWord(volumeMessage, "volume");
  int volumeInInt = cleanedVolume.toInt();
  if (volumeInInt >= 0 && volumeInInt <= 30)
  {
    settings::setVolume(volumeInInt);
    delay(100);
    player.volume(volumeInInt);
    return true;
  }
  else
  {
    return false;
  }
  return false;
}

bool azanSetter(String azanStateFromTelegram)
{
  azanStateFromTelegram.toUpperCase();
  if (azanStateFromTelegram == "AZAN ON")
  {
    settings::setPrayerEnabled(true);
    return true;
  }
  else if (azanStateFromTelegram == "AZAN OFF")
  {
    settings::setPrayerEnabled(false);
    return true;
  }
  else
  {
    return false;
  }
}