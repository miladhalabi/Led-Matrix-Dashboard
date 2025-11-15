#ifndef _PrayerTime    // Put these two lines at the top of your file.
#define _PrayerTime    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

void isPrayTime(void *parameter);
void prayerTimeFetch(int year, int month, int day);

extern bool AzanTime;
extern String prayTimeArray[5];
extern bool fetchingAPI;
extern bool AzanNow;

#endif