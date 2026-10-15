#include <Arduino.h>
#include <Preferences.h>
Preferences preferences;

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