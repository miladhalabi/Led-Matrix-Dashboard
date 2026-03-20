#include "weather.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <prayerTime/prayerTime.h>

String weatherTemp = "--";
String weatherGlyph = "";

bool fetchWeather() {
    bool success = false;
    if (WiFi.status() == WL_CONNECTED && !fetchingAPI) {
        fetchingAPI = true;
        HTTPClient http;
        
        // Damascus: lat=33.5138, lon=36.2765
        // Using http instead of https to save RAM
        String url = "http://api.open-meteo.com/v1/forecast?latitude=33.5138&longitude=36.2765&current=temperature_2m,weather_code,is_day";
        
        http.begin(url);
        int httpResponseCode = http.GET();
        
        Serial.print("Weather HTTP Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Free Heap before parse: ");
        Serial.println(ESP.getFreeHeap());

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Weather Payload: " + payload);
            
            DynamicJsonDocument doc(4096);
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                float temp = doc["current"]["temperature_2m"];
                int code = doc["current"]["weather_code"];
                int isDay = doc["current"]["is_day"];
                
                weatherTemp = String((int)round(temp)) + "C";
                
                // Map WMO Weather Codes to custom glyphs
                // 0: Clear sky -> \x03 (Sun) or \x06 (Moon)
                
                if (code == 0) {
                    weatherGlyph = (isDay == 1) ? "\x03" : "\x06"; // Sun or Moon
                } else if (code >= 1 && code <= 3) {
                    weatherGlyph = "\x04"; // Cloud
                } else if (code >= 45 && code <= 48) {
                    weatherGlyph = "\x04"; // Cloud (Fog)
                } else if (code >= 51) {
                    weatherGlyph = "\x05"; // Rain/Storm
                } else {
                    weatherGlyph = "";
                }
                
                Serial.print("Weather updated: ");
                Serial.print(weatherTemp);
                Serial.print(" Code: ");
                Serial.println(code);
                success = true;
            } else {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.print("Weather fetch error: ");
            Serial.println(httpResponseCode);
        }
        http.end();
        fetchingAPI = false;
    }
    return success;
}
