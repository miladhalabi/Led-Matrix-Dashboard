#include <Arduino.h>
#include <ESP32Encoder.h>
#include <time/time.h>
#include <RTClib.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <display/display.h>
#include <utils/utils.h>
#include <flashUtils/flashUtils.h>
#include <converters/converters.h>
#define PRESS_PIN 32

ESP32Encoder encoder;

int newestPos = 0;
int brightness;
int AlarmVolume = 20;
int AlarmMusic = 1;
String alarmState = "";
String prayState = "";
int hours = 0, minutes = 0, day = 0, month = 0, year = 0, amPm = 1;

void handleEncoder()
{
  enum State
  {
    MAIN_MENU,
    SET_TIME,
    SET_ALARM,
    SET_BRIGHTNESS,
    SET_ALARM_STATUS,
    SET_ALARM_VOL,
    SET_MUSIC,
    SET_PRAY_STATE,
    EXIT
  };

  enum AlarmStatus
  {
    ALARM_OFF,
    ALARM_ON
  };

  State currentState = MAIN_MENU;
  int menuOption = 0;
  int pressCounter = 0;
  String isPMorAM = "";
  int alarmHours = 0, alarmMinutes = 0;
  AlarmStatus alarmStatus = ALARM_OFF;
  int veryoldPos = 0;
  DateTime now = rtc.now();
  hours = now.hour();
  minutes = now.minute();
  day = now.day();
  month = now.month();
  year = now.year() - 2000;
  if (hours > 12)
  {
    hours = hours - 12;
  }
  myDisplay.print("Menu");
  delay(1000);
  int i = 0;
  for (;;)
  {

    static long oldPosition = -999;
    long newPosition = encoder.getCount();
    if (longpress(PRESS_PIN, 1000, 0))
    {
      encoder.clearCount();
      pressCounter++;
      oldPosition = -999;
      newPosition = 1;
      veryoldPos = 0;
      if (currentState == MAIN_MENU)
      {
        switch (menuOption)
        {
        case 0:
          currentState = SET_TIME;
          myDisplay.print("T.set");
          delay(1000);
          break;
        case 1:
          currentState = SET_ALARM;
          myDisplay.print("A.set");
          delay(1000);
          break;
        case 2:
          currentState = SET_BRIGHTNESS;
          myDisplay.print("B.set");
          delay(1000);
          break;
        case 3:
          currentState = SET_ALARM_STATUS;
          myDisplay.print("A.st");
          delay(1000);
          break;
        case 4:
          currentState = SET_ALARM_VOL;
          myDisplay.print("V.st");
          delay(1000);
          break;
        case 5:
          currentState = SET_MUSIC;
          myDisplay.print("M.st");
          delay(1000);
          break;
        case 6:
          currentState = SET_PRAY_STATE;
          myDisplay.print("P.st");
          delay(1000);
          break;
        case 7:
          currentState = EXIT;
          myDisplay.print("EXIT");
          delay(1000);
          // references.end(); // Close the preferences before exiting
          return; // Exit the function
        }
      }
    }

    if (newPosition != oldPosition)
    {
      oldPosition = newPosition;

      if (currentState == MAIN_MENU)
      {
        newPosition = constrain(newPosition, 0, 7);
        menuOption = constrain(newPosition, 0, 7);
        switch (menuOption)
        {
        case 0:
          myDisplay.print("Time");
          break;
        case 1:
          myDisplay.print("Alarm");
          break;
        case 2:
          myDisplay.print("Bright");
          break;
        case 3:
          myDisplay.print("AlStat");
          break;
        case 4:
          myDisplay.print("Volume");
          break;
        case 5:
          myDisplay.print("Music");
          break;
        case 6:
          myDisplay.print("PrStat");
          break;
        case 7:
          myDisplay.print("Exit");
          break;
        }
      }
      else if (currentState == SET_TIME)
      {
        if (pressCounter == 1)
        {
          if (newPosition > veryoldPos)
          {
            Serial.println("increase");
            hours++;
          }
          else if (newPosition < veryoldPos)
          {
            Serial.println("decrease");
            hours--;
          }
          veryoldPos = newPosition;

          hours = constrain(hours, 1, 12);
          // hours = newPosition;
          myDisplay.print("Hr>" + longToString(hours));
        }
        else if (pressCounter == 2)
        {
          newPosition = constrain(newPosition, 1, 100);
          amPm = newPosition;
          isPMorAM = (amPm % 2 == 0) ? "AM" : "PM";
          myDisplay.print(">" + isPMorAM);
        }
        else if (pressCounter == 3)
        {
          Serial.println("veryOld");
          Serial.println(veryoldPos);
          Serial.println("newPostion");
          Serial.println(newPosition);
          if (newPosition > veryoldPos)
          {
            Serial.println("increase");
            minutes++;
          }
          else if (newPosition < veryoldPos)
          {
            Serial.println("decrease");
            minutes--;
          }
          veryoldPos = newPosition;
          minutes = constrain(minutes, 1, 59);
          //minutes = newPosition;
          myDisplay.print("Min>" + longToString(minutes));
        }
        else if (pressCounter == 4)
        {
          if (newPosition > veryoldPos)
          {
            Serial.println("increase");
            day++;
          }
          else if (newPosition < veryoldPos)
          {
            Serial.println("decrease");
            day--;
          }
          veryoldPos = newPosition;
          day = constrain(day, 1, 31);
          myDisplay.print("Day>" + longToString(day));
        }
        else if (pressCounter == 5)
        {
          if (newPosition > veryoldPos)
          {
            Serial.println("increase");
            month++;
          }
          else if (newPosition < veryoldPos)
          {
            Serial.println("decrease");
            month--;
          }
          veryoldPos = newPosition;
          month = constrain(month, 1, 12);
          myDisplay.print("Mon>" + longToString(month));
        }
        else if (pressCounter == 6)
        {
          if (newPosition > veryoldPos)
          {
            Serial.println("increase");
            year++;
          }
          else if (newPosition < veryoldPos)
          {
            Serial.println("decrease");
            year--;
          }
          veryoldPos = newPosition;
          year = constrain(year, 1, 99);
          myDisplay.print(">" + longToString(2000 + year));
        }
        else if (pressCounter == 7)
        {
          int hour24, minute24;
          convertTo24Hour(hours, minutes, isPMorAM, hour24, minute24);
          rtc.adjust(DateTime(2000 + year, month, day, hour24, minutes, 0));
          myDisplay.print("EXIT");
          delay(1000);
          currentState = MAIN_MENU;
          pressCounter = 0;
        }
      }
      else if (currentState == SET_ALARM)
      {
        if (pressCounter == 1)
        {
          newPosition = constrain(newPosition, 1, 12);
          alarmHours = newPosition;
          myDisplay.print("Hr>" + longToString(alarmHours));
        }
        else if (pressCounter == 2)
        {
          newPosition = constrain(newPosition, 1, 100);
          amPm = newPosition;
          isPMorAM = (amPm % 2 == 0) ? "AM" : "PM";
          myDisplay.print(">" + isPMorAM);
        }
        else if (pressCounter == 3)
        {
          newPosition = constrain(newPosition, 0, 59);
          alarmMinutes = newPosition;
          myDisplay.print("Min>" + longToString(alarmMinutes));
        }
        else if (pressCounter == 4)
        {
          int hour24Alarm, minute24Alarm;
          convertTo24Hour(alarmHours, alarmMinutes, isPMorAM, hour24Alarm, minute24Alarm);
          StoreData("alarmHour", hour24Alarm);
          StoreData("alarmMins", minute24Alarm);
          AlarmHour = ReadData("alarmHour");
          AlarmMins = ReadData("alarmMins");
          StoreDataString("AMpm", isPMorAM);
          myDisplay.print("EXIT");
          delay(1000);
          currentState = MAIN_MENU;
          pressCounter = 0;
        }
      }
      else if (currentState == SET_BRIGHTNESS)
      {
        if (pressCounter == 1)
        {
          Serial.println("here again");
          newPosition = constrain(newPosition, 1, 8);
          brightness = newPosition;
          myDisplay.setIntensity(brightness);
          myDisplay.print("Bri>" + longToString(brightness));
          Serial.println("state");
          Serial.println(currentState);
          Serial.println("presscounter");
          Serial.println(pressCounter);
          Serial.println("new pos");
          Serial.println(newPosition);
          Serial.println("to store: ");
          Serial.println(brightness);
        }
        else if (pressCounter == 2)
        {
          StoreData("br", brightness);
          myDisplay.setIntensity(brightness);
          Serial.println("stored: ");
          Serial.println(ReadData("br"));
          myDisplay.print("EXIT");
          delay(1000);
          encoder.clearCount();
          currentState = MAIN_MENU;
          pressCounter = 0;
          delay(200);
        }
      }
      else if (currentState == SET_ALARM_STATUS)
      {
        if (pressCounter == 1)
        {
          newPosition = constrain(newPosition, 1, 100);
          alarmState = (newPosition % 2 == 0) ? "ON" : "OFF";
          myDisplay.print(alarmState);
        }
        else if (pressCounter == 2)
        {
          StoreDataString("almState", alarmState);
          alarmState = ReadDataString("almState");
          myDisplay.print("EXIT");
          delay(1000);
          currentState = MAIN_MENU;
          pressCounter = 0;
        }
      }
      else if (currentState == SET_ALARM_VOL)
      {
        if (pressCounter == 1)
        {
          newPosition = constrain(newPosition, 0, 30);
          AlarmVolume = newPosition;
          myDisplay.print("Vol>" + longToString(AlarmVolume));
        }
        else if (pressCounter == 2)
        {
          StoreData("AlarmVolume", AlarmVolume);
          AlarmVolume = ReadData("AlarmVolume");
          //player.reset();
          delay(100);
          player.volume(AlarmVolume);
          //printDetail(player.readType(), player.read());
          myDisplay.print("EXIT");
          delay(1000);
          currentState = MAIN_MENU;
          pressCounter = 0;
        }
      }
      else if (currentState == SET_MUSIC)
      {
        if (pressCounter == 1)
        {
          newPosition = constrain(newPosition, 0, 30);
          AlarmMusic = newPosition;
          myDisplay.print(">" + longToString(AlarmMusic));
          if (newPosition != newestPos)
          {
            Serial.print("newPosition : ");
            Serial.println(newPosition);
            Serial.print("newestPos : ");
            Serial.println(newestPos);
            delay(50);
            //printDetail(player.readType(), player.read());
            Serial.println("been here");
            player.stop();
            delay(100);
            player.play(AlarmMusic);
            delay(50);
          }
          newestPos = newPosition;
        }
        else if (pressCounter == 2)
        {
          player.stop();
          //player.reset();
          StoreData("AlarmMusic", AlarmMusic);
          AlarmMusic = ReadData("AlarmMusic");
          myDisplay.print("EXIT");
          delay(1000);
          currentState = MAIN_MENU;
          pressCounter = 0;
        }
      }
      else if (currentState == SET_PRAY_STATE)
      {
        if (pressCounter == 1)
        {
          newPosition = constrain(newPosition, 1, 100);
          prayState = (newPosition % 2 == 0) ? "ON" : "OFF";
          myDisplay.print(prayState);
        }
        else if (pressCounter == 2)
        {
          StoreDataString("prayState", prayState);
          prayState = ReadDataString("prayState");
          myDisplay.print("EXIT");
          delay(1000);
          currentState = MAIN_MENU;
          pressCounter = 0;
        }
      }
      
    }
    esp_task_wdt_reset(); // Reset the watchdog timer
  }
}
void handleEncoderTask(void *parameter)
{
  int oldPosition = -999;
  for (;;)
  {
    int newPosition = encoder.getCount();
    if (oldPosition != newPosition)
    {
      Serial.println(newPosition);
      oldPosition = newPosition;
    }
    esp_task_wdt_reset();                 // Reset the watchdog timer
    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for stability
  }
}