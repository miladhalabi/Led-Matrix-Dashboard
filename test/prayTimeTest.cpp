#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "system";
const char *password = "m12345678";
void prayerTime(String &fajr , String &dhuhr , String &asr , String &maghrip , String &isha)
{
  String day = String(1);
  String month = String(1);
  String year = String(2024);
  String serverName = "https://api.aladhan.com/v1/timingsByCity/" + day + "-" + month + "-" + year +"?city=Damascus&country=Syria&method=8";
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String serverPath = serverName;
    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      const char *timings[] = {
          "Fajr", "Sunrise", "Dhuhr", "Asr", "Sunset", "Maghrib", "Isha", "Imsak", "Midnight", "Firstthird", "Lastthird"};
      String Fajr = (doc["data"]["timings"]["Fajr"]);
      String Dhuhr = doc["data"]["timings"]["Dhuhr"];
      String Asr = doc["data"]["timings"]["Asr"];
      String Maghrib = doc["data"]["timings"]["Maghrib"];
      String Isha = doc["data"]["timings"]["Isha"];
      fajr = Fajr;
      dhuhr = Dhuhr;
      asr = Asr;
      maghrip = Maghrib;
      isha = Isha;
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    String Fajr;
    String Dhuhr;
    String Asr;
    String Maghrib;
    String Isha;
    prayerTime(Fajr , Dhuhr , Asr , Maghrib , Isha);
    Serial.println(Fajr);
    Serial.println(Dhuhr);
    Serial.println(Asr);
    Serial.println(Maghrib);
    Serial.println(Isha);
    lastTime = millis();
  }
}