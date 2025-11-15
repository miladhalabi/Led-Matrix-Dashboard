#include <Arduino.h>
#include <esp_task_wdt.h>
#include <./converters/converters.h>

void resetEveryTwoHours()
{
  static unsigned long lastResetTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastResetTime >= 7200000){
    esp_restart();
  }
}

void monitorFreeHeapMemory(void *parameters)
{
  for (;;)
  {
    Serial.printf("Free heap memory: %d bytes\n", ESP.getFreeHeap());
    esp_task_wdt_reset();                   // Reset the watchdog timer
    vTaskDelay(60000 / portTICK_PERIOD_MS); // Check every 60 seconds
  }
}

bool longpress(int pin, int dtime, int state)
{
  if (digitalRead(pin) == nat(state))
  {
    return false;
  }
  int i = 0;
  while (digitalRead(pin) == state && i <= 10)
  {
    delay(dtime / 10);
    if (i == 10)
    {
      return true;
    }
    i++;
  }
  return false;
}

int lenFull(String arr[] , int Len)
{
  int counter = 0;
  for(int i = 0 ; i <= Len ; i++)
  {
    if(arr[i] == "")
    {
      return counter;
    }
    counter++;
  }
  return counter;
}