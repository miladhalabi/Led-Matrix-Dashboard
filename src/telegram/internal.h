#ifndef _telegram_internal
#define _telegram_internal

#include <Arduino.h>
#include <AsyncTelegram2.h>

// Shared by the files in src/telegram/ only. The rest of the firmware uses telegram.h.

extern AsyncTelegram2 bot;

// telegram.cpp
bool isOwner(const TBMessage &msg);

// commands.cpp - plain text commands ("alarm,07:00:am", "ring", ...).
// Returns false when `text` is not a command, so the caller can treat it as a message to scroll.
bool runTextCommand(TBMessage &msg, const String &text);
void setScrollMessage(const String &text);

// menus.cpp - inline keyboards and their button callbacks.
void showMainMenu(TBMessage &msg, bool edit = false);
void runCallback(TBMessage &msg, const String &data);
bool takeAwaitingMessage(); // true once after the user pressed the "Msg" button

// ota.cpp - firmware update from a .bin sent to the bot.
void rememberFirmware(const String &filePath);
bool hasFirmware();
void startOtaUpdate(TBMessage &msg);

#endif
