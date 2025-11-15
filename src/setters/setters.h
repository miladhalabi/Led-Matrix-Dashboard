#ifndef _setters    // Put these two lines at the top of your file.
#define _setters    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

bool alarmSetter(String timeToAlarm);
bool volumeSetter(String volumeMessage);
bool timeSetter(String timeMessgae);
bool dateSetter(String dateMessge);
bool azanSetter(String azanStateFromTelegram);

#endif