#ifndef _shared_string
#define _shared_string

#include <Arduino.h>

/**
 * A String that one task can replace while other tasks read it.
 * A plain global String is not safe for that: a read that overlaps a write can see freed memory.
 * get() returns a copy, so keep it in a local variable while you use it.
 */
class SharedString
{
public:
  explicit SharedString(const char *initial = "") : value_(initial), lock_(xSemaphoreCreateMutex()) {}

  String get() const
  {
    xSemaphoreTake(lock_, portMAX_DELAY);
    String copy = value_;
    xSemaphoreGive(lock_);
    return copy;
  }

  void set(const String &text)
  {
    xSemaphoreTake(lock_, portMAX_DELAY);
    value_ = text;
    xSemaphoreGive(lock_);
  }

private:
  String value_;
  SemaphoreHandle_t lock_;
};

#endif
