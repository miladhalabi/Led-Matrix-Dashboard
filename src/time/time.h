#ifndef _time    // Put these two lines at the top of your file.
#define _time    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <RTClib.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"


extern RTC_DS3231 rtc;
extern DFRobotDFPlayerMini player;

extern String timeNow, dateNow;
extern int AlarmHour;
extern int AlarmMins;


void handleTimeTask(void *parameter);
void HandleAlarmTask();
void alarmRing(void *parameter);


#endif