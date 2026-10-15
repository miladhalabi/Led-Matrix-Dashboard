#include "rtcClock.h"

namespace
{
  RTC_DS3231 rtc;
  SemaphoreHandle_t lock = nullptr;

  struct Guard
  {
    Guard()
    {
      if (lock)
        xSemaphoreTake(lock, portMAX_DELAY);
    }
    ~Guard()
    {
      if (lock)
        xSemaphoreGive(lock);
    }
  };
}

bool rtcBegin()
{
  if (!lock)
    lock = xSemaphoreCreateMutex();
  Guard guard;
  return rtc.begin();
}

DateTime rtcNow()
{
  Guard guard;
  return rtc.now();
}

void rtcAdjust(const DateTime &dateTime)
{
  Guard guard;
  rtc.adjust(dateTime);
}
