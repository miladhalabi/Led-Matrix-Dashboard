#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time/time.h>
#include <flashUtils/flashUtils.h>
#include <utils/utils.h>
#include <parsers/parsers.h>
#include <encoder/encoder.h>
#include <weather/weather.h>

const int prayTimeArrayLen = 5;
String prayTimeArray[prayTimeArrayLen];
bool AzanTime = false;
bool AzanNow = false;
bool fetchOnBoot = false;
bool fetchingAPI = false;

bool prayerTimeFetch(int year, int month, int day)
{
  bool success = false;
  fetchingAPI = true;
  vTaskDelay(10 / portTICK_PERIOD_MS);
  // Use http instead of https to save RAM (SSL overhead)
  String serverName = "http://api.aladhan.com/v1/timingsByCity/" + String(day) + "-" + String(month) + "-" + String(year) + "?city=Damascus&country=Syria&method=8";

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverName.c_str());

    // Perform the HTTP GET request
    int httpResponseCode = http.GET();
    
    Serial.print("Prayer HTTP Code: ");
    Serial.println(httpResponseCode);
    Serial.print("Free Heap before parse: ");
    Serial.println(ESP.getFreeHeap());

    // If the response is successful
    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      Serial.println("Prayer Payload: " + payload);
      
      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Extract prayer times from the JSON response
        String Fajr = (doc["data"]["timings"]["Fajr"]);
        String Dhuhr = doc["data"]["timings"]["Dhuhr"];
        String Asr = doc["data"]["timings"]["Asr"];
        String Maghrib = doc["data"]["timings"]["Maghrib"];
        String Isha = doc["data"]["timings"]["Isha"];
        
        // Store the fetched prayer times
        prayTimeArray[0] = Fajr;
        prayTimeArray[1] = Dhuhr;
        prayTimeArray[2] = Asr;
        prayTimeArray[3] = Maghrib;
        prayTimeArray[4] = Isha;
        storeStringArray("prayTimeArray", prayTimeArray, prayTimeArrayLen);

        Serial.println("Prayer times fetched successfully.");
        success = true;
      } else {
        Serial.print("Prayer deserializeJson() failed: ");
        Serial.println(error.c_str());
      }
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    
    // End the HTTP connection
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }

  fetchingAPI = false;
  return success;
}

unsigned long lastRTCUpdate = 0; 
const long rtcUpdateInterval = 1000; // Update RTC every 1000ms (1 second)
DateTime now;  // Cached time from rtc.now()

void updateCachedTime()
{
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastRTCUpdate >= rtcUpdateInterval)
  {
    lastRTCUpdate = currentMillis;
    now = rtc.now();  // Only update 'now' once per second
  }
}

bool isTimeForAzan(int prayerHour, int prayerMinute)
{
  // Check if it's the exact time of the prayer
  return (prayerHour == now.hour() && prayerMinute == now.minute() && now.second() == 0);
}

void isPrayTime(void *parameter)
{
  for(;;)
  {
    updateCachedTime();

    if (WiFi.status() == WL_CONNECTED)
    {
      bool isTopOfHour = (now.minute() == 0 && now.second() == 0);
      if (fetchOnBoot == false || isTopOfHour)
      {
        loadStringArray("prayTimeArray", prayTimeArray, prayTimeArrayLen);
        bool pSuccess = prayerTimeFetch(now.year(), now.month(), now.day());
        bool wSuccess = fetchWeather();

        if (pSuccess && wSuccess)
        {
          fetchOnBoot = true;
          // Wait so we don't trigger multiple times in the same second
          vTaskDelay(2000 / portTICK_PERIOD_MS);
          updateCachedTime();
        }
        else
        {
          Serial.println("Internet fetch failed, retrying in 30 seconds...");
          for (int i = 0; i < 30; i++) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_task_wdt_reset();
          }
          continue; // Try again immediately after wait
        }
      }
    }

    if(prayState == "ON")
    {
      // Parse prayer times
      int fajrHour, fajrMinute, dhuhrHour, dhuhrMinute, asrHour, asrMinute, maghribHour, maghribMinute, ishaHour, ishaMinute;
      
      parseTime24(prayTimeArray[0], fajrHour, fajrMinute);
      parseTime24(prayTimeArray[1], dhuhrHour, dhuhrMinute);
      parseTime24(prayTimeArray[2], asrHour, asrMinute);
      parseTime24(prayTimeArray[3], maghribHour, maghribMinute);
      parseTime24(prayTimeArray[4], ishaHour, ishaMinute);

      // Check for any prayer time
      if (isTimeForAzan(fajrHour, fajrMinute) ||
          isTimeForAzan(dhuhrHour, dhuhrMinute) ||
          isTimeForAzan(asrHour, asrMinute) ||
          isTimeForAzan(maghribHour, maghribMinute) ||
          isTimeForAzan(ishaHour, ishaMinute))
      {
        AzanTime = true;
        if(AzanTime)
        {
          Serial.println("++++++++++++++++++");
          AzanNow = true;
          vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        AzanTime = false;
      }
    }
    
    esp_task_wdt_reset(); 
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
