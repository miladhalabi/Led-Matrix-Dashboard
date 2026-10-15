#include <Arduino.h>
#include <MD_Parola.h>
#include <esp_task_wdt.h>
#include <time/time.h>
#include <telegram/telegram.h>
#include <utils/utils.h>
#include <encoder/encoder.h>
#include "pages.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 5
#define PRESS_PIN 32

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

unsigned long lastPageTime = 0;
const unsigned long PAGE_INTERVAL_MS = 10000; // the clock is shown in between pages
// Parola keeps the pointer it is given while it animates, so the text lives in buffers of our own.
static char clockText[32];
static char otaText[32];
uint8_t customChar[] = { 0x07, 0x40, 0x7C, 0x42, 0xC3, 0x42, 0x7C, 0x40 };
uint8_t customChar1[] = {0x08 , 0x0E,0x11,0x21,0x42,0x42,0x21,0x11,0x0E};
uint8_t sunGlyph[] = {0x08, 0x81,0x5A,0x24,0x42,0x42,0x24,0x5A,0x81};
uint8_t cloudGlyph[] = {0x08,0x70,0x58,0x44,0x44,0x44,0x44,0x58,0x70};
uint8_t rainGlyph[] = {0x08 ,0x1C,0x96,0x51,0x11,0x91,0x51,0x16,0x1C};
uint8_t moonGlyph[] = {0x08, 0x80,0x80,0xC0,0xC0,0xE0,0x70,0x3C,0x1F};
void handleDisplayTask(void *parameter)
{
  for (;;)
  {
    // If OTA update is in progress, show the OTA status text instead of normal display
    if (isOTAUpdating) {
      if (myDisplay.displayAnimate()) {
        strlcpy(otaText, otaProgressText.get().c_str(), sizeof(otaText));
        myDisplay.displayScroll(otaText, PA_CENTER, PA_SCROLL_LEFT, 50);
      }
      esp_task_wdt_reset();
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }

    if (millis() - lastPageTime >= PAGE_INTERVAL_MS)
    {
      if (myDisplay.displayAnimate())
      {
        showNextPage();
        delay(100);
        myDisplay.displayReset();
        lastPageTime = millis();
      }
    }
    else
    {
      delay(5);
      if (myDisplay.displayAnimate())
      {
        strlcpy(clockText, timeNow.get().c_str(), sizeof(clockText));
        myDisplay.displayText(clockText, PA_CENTER, myDisplay.getSpeed(), 10000, PA_SCROLL_DOWN, PA_SCROLL_DOWN);
        delay(100);
        myDisplay.displayReset();
      }
    }
    if (longpress(PRESS_PIN, 1000, 0))
    {
      handleEncoder();
    }
    HandleAlarmTask();
    esp_task_wdt_reset();                // Reset the watchdog timer
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to yield
  }
}