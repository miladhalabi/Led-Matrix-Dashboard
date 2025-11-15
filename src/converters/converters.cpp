#include <Arduino.h>

int nat(int number)
{
  return number == 0 ? 1 : 0;
}

String longToString(long number)
{
  char buffer[20];
  sprintf(buffer, "%ld", number);
  return String(buffer);
}

void convertTo24Hour(int hour12, int minute, const String &period, int &hour24, int &minute24)
{
  hour24 = hour12;
  minute24 = minute;
  if (period == "PM" && hour12 != 12)
  {
    hour24 += 12;
  }
  if (period == "AM" && hour12 == 12)
  {
    hour24 = 0;
  }
}
