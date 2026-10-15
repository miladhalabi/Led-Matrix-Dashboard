#ifndef _settings
#define _settings

#include <Arduino.h>

/**
 * Everything the user can configure that must survive a reboot.
 *
 * To add a setting: add a field here (with its default), then load it in settings::load(),
 * and add a setter in settings.cpp that validates the value and saves it.
 */
struct Settings
{
  uint8_t alarmHour = 0;     // 0-23
  uint8_t alarmMinute = 0;   // 0-59
  bool alarmEnabled = false;
  bool prayerEnabled = false;
  uint8_t volume = 20;       // 0-30 (DFPlayer range)
  uint8_t music = 1;         // alarm track number on the SD card
  uint8_t brightness = 4;    // 1-8 (matrix intensity)
};

namespace settings
{
  /** Reads everything from flash. Call once in setup(), before any task starts. */
  void load();

  /** Thread-safe copy of the current values. Cheap; call it wherever a value is needed. */
  Settings get();

  // Setters validate (clamp) the value, update the in-memory copy and save it to flash.
  void setAlarmTime(int hour, int minute);
  void setAlarmEnabled(bool enabled);
  void setPrayerEnabled(bool enabled);
  void setVolume(int volume);
  void setMusic(int track);
  void setBrightness(int brightness);

  /** Last text the user sent to scroll on the matrix. */
  String lastMessage();
  void setLastMessage(const String &message);
}

#endif
