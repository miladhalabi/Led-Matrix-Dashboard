#include <Arduino.h>
#include <Preferences.h>
Preferences preferences;

void StoreData(const char *key, const int val)
{
  preferences.begin("store", false);
  preferences.putInt(key, val);
  preferences.end();
}

int ReadData(const char *val)
{
  preferences.begin("store", false);
  int ret = preferences.getInt(val);
  preferences.end();
  return ret;
}

void StoreDataString(const char *key, const String val)
{
  preferences.begin("store", false);
  preferences.putString(key, val);
  preferences.end();
}

String ReadDataString(const char *val)
{
  preferences.begin("store", false);
  String ret = preferences.getString(val);
  preferences.end();
  return ret;
}

void writeToFlash(const char *key, int valueToStore)
{
  preferences.putInt(key, valueToStore);
  Serial.printf("Stored integer at %s: %d\n", key, valueToStore);
}

int readFromFlash(const char *key)
{
  int storedValue = preferences.getInt(key, 0);
  Serial.printf("Read integer from %s: %d\n", key, storedValue);
  return storedValue;
}

void storeStringArray(String keyPrefix, String arr[], int size)
{
  preferences.begin("store", false);

  for (int i = 0; i < size; i++)
  {
    String key = keyPrefix + String(i);
    preferences.putString(key.c_str(), arr[i]);
  }
  preferences.end();
}

void loadStringArray(String keyPrefix, String arr[], int size)
{
  preferences.begin("store", true);

  for (int i = 0; i < size; i++)
  {
    String key = keyPrefix + String(i);
    arr[i] = preferences.getString(key.c_str(), "");
  }
  preferences.end(); // Close preferences
}