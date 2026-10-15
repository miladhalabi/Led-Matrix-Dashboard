#include "settings.h"
#include <Preferences.h>

namespace settings
{
  namespace
  {
    // Same flash namespace and keys the firmware has always used, so a device that is
    // updated keeps its saved settings.
    const char *const NAMESPACE = "store";
    const char *const KEY_ALARM_HOUR = "alarmHour";
    const char *const KEY_ALARM_MINUTE = "alarmMins";
    const char *const KEY_ALARM_STATE = "almState"; // "ON" / "OFF"
    const char *const KEY_PRAYER_STATE = "prayState"; // "ON" / "OFF"
    const char *const KEY_VOLUME = "AlarmVolume";
    const char *const KEY_MUSIC = "AlarmMusic";
    const char *const KEY_BRIGHTNESS = "br";
    const char *const KEY_MESSAGE = "LastMessge";

    Settings current;
    String message;
    SemaphoreHandle_t lock = nullptr;
    Preferences prefs;

    // Held while touching `current`, `message` or flash, so tasks never see half-written values.
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

    void saveInt(const char *key, int value)
    {
      prefs.begin(NAMESPACE, false);
      prefs.putInt(key, value);
      prefs.end();
    }

    void saveString(const char *key, const String &value)
    {
      prefs.begin(NAMESPACE, false);
      prefs.putString(key, value);
      prefs.end();
    }

    void saveFlag(const char *key, bool on) { saveString(key, on ? "ON" : "OFF"); }
  }

  void load()
  {
    if (!lock)
      lock = xSemaphoreCreateMutex();
    Guard guard;

    prefs.begin(NAMESPACE, false);
    current.alarmHour = constrain(prefs.getInt(KEY_ALARM_HOUR, 0), 0, 23);
    current.alarmMinute = constrain(prefs.getInt(KEY_ALARM_MINUTE, 0), 0, 59);
    current.alarmEnabled = prefs.getString(KEY_ALARM_STATE, "OFF") == "ON";
    current.prayerEnabled = prefs.getString(KEY_PRAYER_STATE, "OFF") == "ON";
    current.volume = constrain(prefs.getInt(KEY_VOLUME, 20), 0, 30);
    current.music = constrain(prefs.getInt(KEY_MUSIC, 1), 0, 255);
    current.brightness = constrain(prefs.getInt(KEY_BRIGHTNESS, 4), 1, 8);
    message = prefs.getString(KEY_MESSAGE, "");
    prefs.end();
  }

  Settings get()
  {
    Guard guard;
    return current;
  }

  void setAlarmTime(int hour, int minute)
  {
    Guard guard;
    current.alarmHour = constrain(hour, 0, 23);
    current.alarmMinute = constrain(minute, 0, 59);
    saveInt(KEY_ALARM_HOUR, current.alarmHour);
    saveInt(KEY_ALARM_MINUTE, current.alarmMinute);
  }

  void setAlarmEnabled(bool enabled)
  {
    Guard guard;
    current.alarmEnabled = enabled;
    saveFlag(KEY_ALARM_STATE, enabled);
  }

  void setPrayerEnabled(bool enabled)
  {
    Guard guard;
    current.prayerEnabled = enabled;
    saveFlag(KEY_PRAYER_STATE, enabled);
  }

  void setVolume(int volume)
  {
    Guard guard;
    current.volume = constrain(volume, 0, 30);
    saveInt(KEY_VOLUME, current.volume);
  }

  void setMusic(int track)
  {
    Guard guard;
    current.music = constrain(track, 0, 255);
    saveInt(KEY_MUSIC, current.music);
  }

  void setBrightness(int brightness)
  {
    Guard guard;
    current.brightness = constrain(brightness, 1, 8);
    saveInt(KEY_BRIGHTNESS, current.brightness);
  }

  String lastMessage()
  {
    Guard guard;
    return message;
  }

  void setLastMessage(const String &text)
  {
    Guard guard;
    message = text;
    saveString(KEY_MESSAGE, message);
  }
}
