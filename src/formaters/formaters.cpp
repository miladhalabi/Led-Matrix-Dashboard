#include <Arduino.h>

String formatTime12Hour(int hour, int minute, bool &isPM)
{
  isPM = (hour >= 12);
  if (hour > 12)
    hour -= 12;
  if (hour == 0)
    hour = 12;
  char timeString[6];
  sprintf(timeString, "%d:%02d", hour, minute);
  return String(timeString);
}
