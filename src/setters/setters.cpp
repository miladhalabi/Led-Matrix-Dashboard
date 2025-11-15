#include <Arduino.h>
#include <parsers/parsers.h>
#include <encoder/encoder.h>
#include <flashUtils/flashUtils.h>
#include <converters/converters.h>
#include <time/time.h>
#include <encoder/encoder.h>

bool alarmSetter(String timeToAlarm)
{
  timeToAlarm.toUpperCase();
  if (timeToAlarm == "ALARM ON")
  {
    StoreDataString("almState", "ON");
    alarmState = ReadDataString("almState");
    return true;
  }
  else if (timeToAlarm == "ALARM OFF")
  {
    StoreDataString("almState", "OFF");
    alarmState = ReadDataString("almState");
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
      StoreData("alarmHour", hour24Alarm);
      StoreData("alarmMins", minute24Alarm);
      AlarmHour = ReadData("alarmHour");
      AlarmMins = ReadData("alarmMins");
      StoreDataString("AMpm", NightDay);
      return true;
    }
    return false;
  }
  return false;
}

bool timeSetter(String timeMessgae)
{
  DateTime getTimeTelegram = rtc.now();
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
    rtc.adjust(DateTime(yearTelegram, monthTelegram, dayTelegram, nowTelegramHour, nowTelegramMinute));
    return true;
  }
  return false;
}

bool dateSetter(String dateMessge)
{
  DateTime getTimeTelegram = rtc.now();
  String cleanedTimeMessage = removeStartWord(dateMessge, "date");
  Serial.println(cleanedTimeMessage);
  int hourTelegram = getTimeTelegram.hour(), minuteTelegram = getTimeTelegram.minute();
  int yearTelegram = 0, monthTelegram = 0, dayTelegram = 0;
  if (parseDate(cleanedTimeMessage, yearTelegram, monthTelegram, dayTelegram))
  {
    rtc.adjust(DateTime(yearTelegram, monthTelegram, dayTelegram, hourTelegram, minuteTelegram));
    return true;
  }
  return false;
}

bool volumeSetter(String volumeMessage)
{
  String cleanedVolume = removeStartWord(volumeMessage, "volume");
  int volumeInInt = cleanedVolume.toInt();
  if (volumeInInt <= 30)
  {
    StoreData("AlarmVolume", volumeInInt);
    AlarmVolume = ReadData("AlarmVolume");
    delay(100);
    player.volume(AlarmVolume);
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
    StoreDataString("prayState", "ON");
    prayState = ReadDataString("prayState");
    return true;
  }
  else if (azanStateFromTelegram == "AZAN OFF")
  {
    StoreDataString("prayState", "OFF");
    prayState = ReadDataString("prayState");
    return true;    
  }
  else
  {
    return false;
  }
}