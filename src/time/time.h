#ifndef _time    // Put these two lines at the top of your file.
#define _time    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <RTClib.h>
#include <Wire.h>
#include <rtcClock/rtcClock.h>
#include <shared/shared_string.h>
#include "DFRobotDFPlayerMini.h"


extern DFRobotDFPlayerMini player;

extern SharedString timeNow;
extern SharedString dateNow;


void handleTimeTask(void *parameter);
bool HandleAlarmTask(); // true while the alarm screen is showing
void alarmRing(void *parameter);


#endif