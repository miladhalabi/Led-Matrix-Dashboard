#include <Arduino.h>
#include <time/time.h>
#include <setters/setters.h>
#include <formaters/formaters.h>
#include <parsers/parsers.h>
#include <converters/converters.h>
#include <weather/weather.h>
#include <settings/settings.h>
#include <telegram/telegram.h>
#include <version/version.h>
#include "internal.h"

/**
 * Inline keyboards and what their buttons do. To add a button: add a screen or a line to an
 * existing one, then add a line to CALLBACKS with the handler to run when it is pressed.
 */
namespace
{
  // Button identifiers. Telegram limits them to 64 bytes.
  const char *const CB_MAIN = "main";
  const char *const CB_EDIT_MSG = "edit_msg";
  const char *const CB_ALARM_MENU = "alarm_menu";
  const char *const CB_TIME_MENU = "time_menu";
  const char *const CB_VOL_MENU = "vol_menu";
  const char *const CB_VOL_PREFIX = "vol_"; // followed by the volume, e.g. "vol_15"
  const char *const CB_AZAN_TOGGLE = "azan_toggle";
  const char *const CB_REFRESH = "refresh";
  const char *const CB_WEATHER_REF = "weather_ref";
  const char *const CB_RING = "ring_bot";
  const char *const CB_ALARM_TOG_M = "alm_tog_m";

  const char *const CB_ALARM_H_UP = "alm_h_u";
  const char *const CB_ALARM_H_DN = "alm_h_d";
  const char *const CB_ALARM_M_UP = "alm_m_u";
  const char *const CB_ALARM_M_DN = "alm_m_d";
  const char *const CB_ALARM_P_TOG = "alm_p_t";
  const char *const CB_ALARM_SAVE = "alm_save";
  const char *const CB_ALARM_TOGGLE = "alm_toggle";

  const char *const CB_TIME_H_UP = "time_h_u";
  const char *const CB_TIME_H_DN = "time_h_d";
  const char *const CB_TIME_M_UP = "time_m_u";
  const char *const CB_TIME_M_DN = "time_m_d";
  const char *const CB_TIME_P_TOG = "time_p_t";
  const char *const CB_TIME_SAVE = "time_save";

  // The alarm / clock being edited with the +/- buttons. Saved only on the save button.
  struct Draft
  {
    int hour = 12;
    int minute = 0;
    bool pm = false;
  } draft;

  bool awaitingMessage = false;

  void drawTimeEditor(InlineKeyboard &kbd, const char *hUp, const char *mUp, const char *hDn, const char *mDn, const char *pmToggle)
  {
    kbd.addButton("H +", hUp, KeyboardButtonQuery);
    kbd.addButton("M +", mUp, KeyboardButtonQuery);
    kbd.addRow();

    char timeStr[20];
    sprintf(timeStr, "%02d : %02d %s", draft.hour, draft.minute, draft.pm ? "PM" : "AM");
    kbd.addButton(timeStr, "none", KeyboardButtonQuery);
    kbd.addRow();

    kbd.addButton("H -", hDn, KeyboardButtonQuery);
    kbd.addButton("M -", mDn, KeyboardButtonQuery);
    kbd.addRow();

    kbd.addButton(draft.pm ? "Switch to AM" : "Switch to PM", pmToggle, KeyboardButtonQuery);
    kbd.addRow();
  }

  void showAlarmMenu(TBMessage &msg)
  {
    InlineKeyboard kbd;
    drawTimeEditor(kbd, CB_ALARM_H_UP, CB_ALARM_M_UP, CB_ALARM_H_DN, CB_ALARM_M_DN, CB_ALARM_P_TOG);
    kbd.addButton(settings::get().alarmEnabled ? "Turn OFF" : "Turn ON", CB_ALARM_TOGGLE, KeyboardButtonQuery);
    kbd.addRow();
    kbd.addButton("✅ SAVE", CB_ALARM_SAVE, KeyboardButtonQuery);
    kbd.addButton("❌ Back", CB_MAIN, KeyboardButtonQuery);
    bot.editMessage(msg, "⏰ *Set Alarm Time*", kbd);
  }

  void showTimeMenu(TBMessage &msg)
  {
    InlineKeyboard kbd;
    drawTimeEditor(kbd, CB_TIME_H_UP, CB_TIME_M_UP, CB_TIME_H_DN, CB_TIME_M_DN, CB_TIME_P_TOG);
    kbd.addButton("✅ UPDATE RTC", CB_TIME_SAVE, KeyboardButtonQuery);
    kbd.addRow();
    kbd.addButton("❌ Back", CB_MAIN, KeyboardButtonQuery);
    bot.editMessage(msg, "🕒 *Set System Time*", kbd);
  }

  void showVolMenu(TBMessage &msg)
  {
    InlineKeyboard kbd;
    const int levels[] = {5, 10, 15, 20, 25, 30};
    for (int i = 0; i < 6; i++)
    {
      kbd.addButton(String(levels[i]).c_str(), (String(CB_VOL_PREFIX) + levels[i]).c_str(), KeyboardButtonQuery);
      if (i % 3 == 2)
        kbd.addRow();
    }
    kbd.addButton("❌ Back", CB_MAIN, KeyboardButtonQuery);
    bot.editMessage(msg, "🔊 *Set Volume*", kbd);
  }

  // Copies the clock into the draft, as 12-hour hour / minute / PM.
  void loadDraft(int hour24, int minute)
  {
    String t12 = formatTime12Hour(hour24, minute, draft.pm);
    String ignored;
    parseTime(t12, draft.hour, draft.minute, ignored);
  }

  void saveDraftAs24Hour(int &hour24, int &minute24)
  {
    convertTo24Hour(draft.hour, draft.minute, draft.pm ? "PM" : "AM", hour24, minute24);
  }

  // A callback handler returns the short text Telegram shows to the user (nullptr = "Updated").
  typedef const char *(*Handler)(TBMessage &msg, const String &data);

  struct Callback
  {
    const char *id;
    bool prefix; // match any button id that starts with `id`
    Handler run;
  };

  const char *refresh(TBMessage &msg, const String &)
  {
    fetchWeather();
    showMainMenu(msg, true);
    return nullptr;
  }

  const char *toggleAzan(TBMessage &msg, const String &)
  {
    settings::setPrayerEnabled(!settings::get().prayerEnabled);
    showMainMenu(msg, true);
    return nullptr;
  }

  const char *toggleAlarmFromMain(TBMessage &msg, const String &)
  {
    settings::setAlarmEnabled(!settings::get().alarmEnabled);
    showMainMenu(msg, true);
    return nullptr;
  }

  const char *toggleAlarmInEditor(TBMessage &msg, const String &)
  {
    settings::setAlarmEnabled(!settings::get().alarmEnabled);
    showAlarmMenu(msg);
    return nullptr;
  }

  const char *ring(TBMessage &, const String &)
  {
    ringSent = true;
    return "Ringing...";
  }

  const char *editMessage(TBMessage &msg, const String &)
  {
    awaitingMessage = true;
    bot.sendMessage(msg, "📝 Send me the new message to scroll:");
    return nullptr;
  }

  const char *openAlarmEditor(TBMessage &msg, const String &)
  {
    const Settings cfg = settings::get();
    loadDraft(cfg.alarmHour, cfg.alarmMinute);
    showAlarmMenu(msg);
    return nullptr;
  }

  const char *alarmHourUp(TBMessage &msg, const String &) { draft.hour = (draft.hour % 12) + 1; showAlarmMenu(msg); return nullptr; }
  const char *alarmHourDown(TBMessage &msg, const String &) { draft.hour = (draft.hour == 1) ? 12 : draft.hour - 1; showAlarmMenu(msg); return nullptr; }
  const char *alarmMinuteUp(TBMessage &msg, const String &) { draft.minute = (draft.minute + 5) % 60; showAlarmMenu(msg); return nullptr; }
  const char *alarmMinuteDown(TBMessage &msg, const String &) { draft.minute = (draft.minute < 5) ? 55 : draft.minute - 5; showAlarmMenu(msg); return nullptr; }
  const char *alarmTogglePm(TBMessage &msg, const String &) { draft.pm = !draft.pm; showAlarmMenu(msg); return nullptr; }

  const char *saveAlarm(TBMessage &msg, const String &)
  {
    int h24, m24;
    saveDraftAs24Hour(h24, m24);
    settings::setAlarmTime(h24, m24);
    bot.sendMessage(msg, "✅ Alarm saved!");
    showMainMenu(msg);
    return nullptr;
  }

  const char *openVolumeMenu(TBMessage &msg, const String &) { showVolMenu(msg); return nullptr; }

  const char *setVolume(TBMessage &msg, const String &data)
  {
    volumeSetter("volume " + data.substring(strlen(CB_VOL_PREFIX)));
    showMainMenu(msg, true);
    return nullptr;
  }

  const char *openTimeEditor(TBMessage &msg, const String &)
  {
    DateTime now = rtcNow();
    loadDraft(now.hour(), now.minute());
    showTimeMenu(msg);
    return nullptr;
  }

  const char *timeHourUp(TBMessage &msg, const String &) { draft.hour = (draft.hour % 12) + 1; showTimeMenu(msg); return nullptr; }
  const char *timeHourDown(TBMessage &msg, const String &) { draft.hour = (draft.hour == 1) ? 12 : draft.hour - 1; showTimeMenu(msg); return nullptr; }
  const char *timeMinuteUp(TBMessage &msg, const String &) { draft.minute = (draft.minute + 1) % 60; showTimeMenu(msg); return nullptr; }
  const char *timeMinuteDown(TBMessage &msg, const String &) { draft.minute = (draft.minute == 0) ? 59 : draft.minute - 1; showTimeMenu(msg); return nullptr; }
  const char *timeTogglePm(TBMessage &msg, const String &) { draft.pm = !draft.pm; showTimeMenu(msg); return nullptr; }

  const char *saveTime(TBMessage &msg, const String &)
  {
    int h24, m24;
    saveDraftAs24Hour(h24, m24);
    DateTime now = rtcNow();
    rtcAdjust(DateTime(now.year(), now.month(), now.day(), h24, m24, 0));
    bot.sendMessage(msg, "✅ System time updated!");
    showMainMenu(msg);
    return nullptr;
  }

  // Exact ids are listed before prefixes, so "vol_menu" is not taken for a "vol_<n>" button.
  const Callback CALLBACKS[] = {
      {CB_REFRESH, false, refresh},
      {CB_MAIN, false, refresh},
      {CB_WEATHER_REF, false, refresh},
      {CB_AZAN_TOGGLE, false, toggleAzan},
      {CB_ALARM_TOG_M, false, toggleAlarmFromMain},
      {CB_RING, false, ring},
      {CB_EDIT_MSG, false, editMessage},

      {CB_ALARM_MENU, false, openAlarmEditor},
      {CB_ALARM_H_UP, false, alarmHourUp},
      {CB_ALARM_H_DN, false, alarmHourDown},
      {CB_ALARM_M_UP, false, alarmMinuteUp},
      {CB_ALARM_M_DN, false, alarmMinuteDown},
      {CB_ALARM_P_TOG, false, alarmTogglePm},
      {CB_ALARM_TOGGLE, false, toggleAlarmInEditor},
      {CB_ALARM_SAVE, false, saveAlarm},

      {CB_VOL_MENU, false, openVolumeMenu},
      {CB_VOL_PREFIX, true, setVolume},

      {CB_TIME_MENU, false, openTimeEditor},
      {CB_TIME_H_UP, false, timeHourUp},
      {CB_TIME_H_DN, false, timeHourDown},
      {CB_TIME_M_UP, false, timeMinuteUp},
      {CB_TIME_M_DN, false, timeMinuteDown},
      {CB_TIME_P_TOG, false, timeTogglePm},
      {CB_TIME_SAVE, false, saveTime},
  };
}

void showMainMenu(TBMessage &msg, bool edit)
{
  const Settings cfg = settings::get();

  InlineKeyboard kbd(BUFFER_MEDIUM);
  kbd.addButton("📝 Msg", CB_EDIT_MSG, KeyboardButtonQuery);
  kbd.addButton("🔔 Ring", CB_RING, KeyboardButtonQuery);
  kbd.addRow();

  kbd.addButton("🕒 Time", CB_TIME_MENU, KeyboardButtonQuery);
  kbd.addButton("🔊 Vol", CB_VOL_MENU, KeyboardButtonQuery);
  kbd.addRow();

  kbd.addButton("⏰ Set", CB_ALARM_MENU, KeyboardButtonQuery);
  kbd.addButton(cfg.alarmEnabled ? "⏰ ON" : "⏰ OFF", CB_ALARM_TOG_M, KeyboardButtonQuery);
  kbd.addRow();

  kbd.addButton(cfg.prayerEnabled ? "🕋 Azan: ON" : "🕋 Azan: OFF", CB_AZAN_TOGGLE, KeyboardButtonQuery);
  kbd.addRow();

  kbd.addButton("🌤 Weather", CB_WEATHER_REF, KeyboardButtonQuery);
  kbd.addButton("🔄 Refresh", CB_REFRESH, KeyboardButtonQuery);

  bool pm = false;
  String almTime = formatTime12Hour(cfg.alarmHour, cfg.alarmMinute, pm);

  String status = "🏠 *LED Matrix Dashboard*\n";
  status += "---------------------------\n";
  status += "💬 *Msg:* \"" + scrolling.get() + "\"\n";
  status += "⏰ *Alarm:* " + almTime + (pm ? " PM" : " AM") + " (" + String(cfg.alarmEnabled ? "ON" : "OFF") + ")\n";
  status += "🕋 *Azan:* " + String(cfg.prayerEnabled ? "ON" : "OFF") + "\n";
  status += "🔊 *Volume:* " + String(cfg.volume) + "\n";
  status += "🌤 *Weather:* " + weatherTemp.get() + "\n";
  status += "🔧 *Firmware:* " + String(FIRMWARE_VERSION);

  if (edit)
    bot.editMessage(msg, status.c_str(), kbd);
  else
    bot.sendMessage(msg, status.c_str(), kbd);
}

void runCallback(TBMessage &msg, const String &data)
{
  const char *answer = nullptr;
  for (const Callback &callback : CALLBACKS)
  {
    const bool match = callback.prefix ? data.startsWith(callback.id) : data == callback.id;
    if (match)
    {
      answer = callback.run(msg, data);
      break;
    }
  }
  bot.endQuery(msg, answer ? answer : "Updated");
}

bool takeAwaitingMessage()
{
  const bool was = awaitingMessage;
  awaitingMessage = false;
  return was;
}
