#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time/time.h>
#include <flashUtils/flashUtils.h>
#include <utils/utils.h>
#include <parsers/parsers.h>
#include <encoder/encoder.h>

const int prayTimeArrayLen = 5;
String prayTimeArray[prayTimeArrayLen];
bool AzanTime = false;
bool AzanNow = false;
bool fetchOnBoot = false;
bool fetchingAPI = false;

void prayerTimeFetch(int year, int month, int day)
{
  fetchingAPI = true;
  vTaskDelay(10 / portTICK_PERIOD_MS);
  String serverName = "https://api.aladhan.com/v1/timingsByCity/" + String(day) + "-" + String(month) + "-" + String(year) + "?city=Damascus&country=Syria&method=8";

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String serverPath = serverName;
    http.begin(serverPath.c_str());

    // Set timeout to 4 seconds (4000 ms)
    //http.setTimeout(4000);  // Timeout for connection and response in milliseconds
    
    // Record the start time
    unsigned long startTime = millis();

    // Perform the HTTP GET request
    int httpResponseCode = http.GET();

    // Check if we've exceeded the 4-second timeout
    if (millis() - startTime >= 6000)
    {
      Serial.println("Error: Request timed out.");
      fetchingAPI = false;
      http.end();
      return;  // Exit the function if timed out
    }

    // If the response is successful
    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
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
    if(prayState == "OFF")
    {
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    else 
    {
      updateCachedTime();

      if ((now.minute() == 0 && now.second() == 0) || fetchOnBoot == false)
      {
        loadStringArray("prayTimeArray", prayTimeArray, prayTimeArrayLen);
        prayerTimeFetch(now.year(), now.month(), now.day());
        fetchOnBoot = true;
      }

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
  }
}
