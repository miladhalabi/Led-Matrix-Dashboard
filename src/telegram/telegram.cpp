#include <Arduino.h>
#include <AsyncTelegram2.h>
#include <esp_task_wdt.h>
#include <wifi/wifi.h>
#include <flashUtils/flashUtils.h>
#include <usersManagement/usersManagement.h>
#include <prayerTime/prayerTime.h>
#include "telegram.h"
#include "internal.h"

#define BOT_REQUEST_DELAY 1000

AsyncTelegram2 bot(client);

bool ringSent = false;
SharedString scrolling;

bool isOwner(const TBMessage &msg)
{
  const String owner = ownerChatId();
  return owner.length() > 0 && String(msg.chatId) == owner;
}

// The owner (set in the config portal) and everyone the owner added with "add,<id>".
static bool isAuthorized(const TBMessage &msg)
{
  if (isOwner(msg))
    return true;
  const String chat = String(msg.chatId);
  for (int i = 0; i < ChatIDsLength; i++)
  {
    if (ChatIDs[i] != "" && chat == ChatIDs[i])
      return true;
  }
  return false;
}

// Only the owner may send firmware.
static void handleDocument(TBMessage &msg)
{
  if (!isOwner(msg))
  {
    bot.sendMessage(msg, "Only the owner can send firmware.");
    return;
  }
  if (!msg.document.file_exists)
    return;

  if (!msg.document.file_path.endsWith(".bin"))
  {
    bot.sendMessage(msg, "Only .bin firmware files are supported.");
    return;
  }

  rememberFirmware(msg.document.file_path);
  String info = "Firmware file received!\n";
  info += "File: " + String(msg.document.file_name) + "\n";
  info += "Size: " + String(msg.document.file_size) + " bytes\n\n";
  info += "Type 'update' to start the firmware update.";
  bot.sendMessage(msg, info);
}

static void handleText(TBMessage &msg)
{
  const String text = msg.text;

  if (takeAwaitingMessage())
  {
    setScrollMessage(text);
    bot.sendMessage(msg, "✅ Message updated!");
    showMainMenu(msg);
    return;
  }

  if (runTextCommand(msg, text))
    return;

  setScrollMessage(text);
  bot.sendMessage(msg, "Message received");
}

void handleNewMessage(TBMessage &msg)
{
  if (!isAuthorized(msg))
    return;

  switch (msg.messageType)
  {
  case MessageQuery:
    runCallback(msg, msg.callbackQueryData);
    break;
  case MessageDocument:
    handleDocument(msg);
    break;
  case MessageText:
    handleText(msg);
    break;
  default: // stickers, photos, ... are ignored
    break;
  }
}

void handleBotTask(void *parameter)
{
  bot.setUpdateTime(BOT_REQUEST_DELAY);

  // Wait for WiFi and for a token to be entered in the config portal
  while (WiFi.status() != WL_CONNECTED || botToken().length() == 0)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  bot.setTelegramToken(botTokenRaw());
  loadStringArray("usersArray", ChatIDs, ChatIDsLength);

  Serial.println("Telegram bot starting...");
  if (bot.begin())
  {
    Serial.println("Telegram bot started");
  }
  else
  {
    Serial.println("Telegram bot failed to start");
  }

  for (;;)
  {
    if (fetchingAPI == false)
    {
      TBMessage msg;
      if (bot.getNewMessage(msg))
      {
        handleNewMessage(msg);
      }
    }

    esp_task_wdt_reset();                // Reset the watchdog timer
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to yield
  }
}
