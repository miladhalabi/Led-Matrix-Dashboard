#include <Arduino.h>
#include <time/time.h>
#include <setters/setters.h>
#include <formaters/formaters.h>
#include <flashUtils/flashUtils.h>
#include <usersManagement/usersManagement.h>
#include <prayerTime/prayerTime.h>
#include <settings/settings.h>
#include <telegram/telegram.h>
#include "internal.h"

/**
 * Text commands. To add one: write a handler below and add a line to COMMANDS.
 * A handler returns true when it dealt with the message (including replying "Invalid values"),
 * or false to let the text be scrolled on the matrix like any other message.
 */
namespace
{
  typedef bool (*Handler)(TBMessage &msg, const String &text);

  struct Command
  {
    const char *name; // the first word of the message (up to a space or comma), or the whole message
    bool ownerOnly;
    Handler run;
  };

  void reply(TBMessage &msg, const String &text) { bot.sendMessage(msg, text); }
  void invalid(TBMessage &msg) { reply(msg, "Invalid values"); }

  // "07:30 PM"
  String clock12(int hour, int minute)
  {
    bool pm = false;
    String t = formatTime12Hour(hour, minute, pm);
    return t + (pm ? " PM" : " AM");
  }

  String firstWord(const String &text)
  {
    for (unsigned int i = 0; i < text.length(); i++)
    {
      if (text[i] == ' ' || text[i] == ',')
        return text.substring(0, i);
    }
    return text;
  }

  bool cmdStart(TBMessage &msg, const String &)
  {
    showMainMenu(msg);
    return true;
  }

  bool cmdRing(TBMessage &msg, const String &)
  {
    ringSent = true;
    reply(msg, "ringing...");
    return true;
  }

  bool cmdUpdate(TBMessage &msg, const String &)
  {
    if (hasFirmware())
      startOtaUpdate(msg);
    else
      reply(msg, "No firmware file received yet. Send a .bin file first.");
    return true;
  }

  bool cmdPrayTimes(TBMessage &msg, const String &)
  {
    String response = "Prayer Times for Today\n";
    response += "-----------------------\n";
    response += "Fajr:    " + prayTimeArray[0] + "\n";
    response += "Dhuhr:   " + prayTimeArray[1] + "\n";
    response += "Asr:     " + prayTimeArray[2] + "\n";
    response += "Maghrib: " + prayTimeArray[3] + "\n";
    response += "Isha:    " + prayTimeArray[4];
    reply(msg, response);
    return true;
  }

  bool cmdAlarm(TBMessage &msg, const String &text)
  {
    if (!alarmSetter(text))
    {
      invalid(msg);
      return true;
    }
    const Settings cfg = settings::get();
    reply(msg, cfg.alarmEnabled ? "alarm on " + clock12(cfg.alarmHour, cfg.alarmMinute) : String("alarm off"));
    return true;
  }

  bool cmdAzan(TBMessage &msg, const String &text)
  {
    if (azanSetter(text))
      reply(msg, String("azan ") + (settings::get().prayerEnabled ? "ON" : "OFF"));
    else
      invalid(msg);
    return true;
  }

  bool cmdVolume(TBMessage &msg, const String &text)
  {
    if (volumeSetter(text))
      reply(msg, "volume on " + String(settings::get().volume));
    else
      invalid(msg);
    return true;
  }

  bool cmdTime(TBMessage &msg, const String &text)
  {
    if (!timeSetter(text))
    {
      invalid(msg);
      return true;
    }
    DateTime now = rtcNow();
    reply(msg, "Time Setted On " + clock12(now.hour(), now.minute()));
    return true;
  }

  bool cmdDate(TBMessage &msg, const String &text)
  {
    if (!dateSetter(text))
    {
      invalid(msg);
      return true;
    }
    DateTime now = rtcNow();
    reply(msg, "Date Setted On " + String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()));
    return true;
  }

  bool cmdAddUser(TBMessage &msg, const String &text)
  {
    if (addUser(ChatIDs, ChatIDsLength, text))
    {
      reply(msg, "User Added");
      loadStringArray("usersArray", ChatIDs, ChatIDsLength);
    }
    else
      invalid(msg);
    return true;
  }

  bool cmdRemoveUser(TBMessage &msg, const String &text)
  {
    if (removeUser(ChatIDs, ChatIDsLength, text))
    {
      reply(msg, "User Removed");
      loadStringArray("usersArray", ChatIDs, ChatIDsLength);
    }
    else
      invalid(msg);
    return true;
  }

  bool cmdListUsers(TBMessage &msg, const String &)
  {
    String users = listUsers(ChatIDs, ChatIDsLength);
    reply(msg, users == "" ? String("No Users") : users);
    return true;
  }

  const Command COMMANDS[] = {
      {"/start", false, cmdStart},
      {"help", false, cmdStart},
      {"ring", false, cmdRing},
      {"praytime", false, cmdPrayTimes},
      {"/praytimes", false, cmdPrayTimes},
      {"alarm", false, cmdAlarm},
      {"azan", false, cmdAzan},
      {"volume", false, cmdVolume},
      {"time", false, cmdTime},
      {"date", false, cmdDate},
      // Changing who may use the bot, or flashing new firmware, is for the owner only.
      {"update", true, cmdUpdate},
      {"add", true, cmdAddUser},
      {"remove", true, cmdRemoveUser},
      {"list users", true, cmdListUsers},
  };
}

void setScrollMessage(const String &text)
{
  scrolling = text;
  settings::setLastMessage(text);
}

bool runTextCommand(TBMessage &msg, const String &text)
{
  const String word = firstWord(text);
  for (const Command &command : COMMANDS)
  {
    if (word != command.name && text != command.name)
      continue;
    if (command.ownerOnly && !isOwner(msg))
    {
      reply(msg, "Only the owner can do that.");
      return true;
    }
    return command.run(msg, text);
  }
  return false;
}
