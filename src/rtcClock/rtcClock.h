#ifndef _rtcClock
#define _rtcClock

#include <Arduino.h>
#include <RTClib.h>

// The DS3231 is on the I2C bus, which must not be used by two tasks at once.
// Every task reads and sets the clock through these functions, which take turns.

bool rtcBegin(); // call once in setup(), before the tasks start
DateTime rtcNow();
void rtcAdjust(const DateTime &dateTime);

#endif
