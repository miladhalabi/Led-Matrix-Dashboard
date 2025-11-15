#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <wifi/wifi.h>
#include <flashUtils/flashUtils.h>
#include <setters/setters.h>
#include <time/time.h>
#include <formaters/formaters.h>
#include <encoder/encoder.h>
#include <usersManagement/usersManagement.h>
#include <prayerTime/prayerTime.h>

#define BOT_TOKEN "6824249584:AAFiZPhLaIB27YrVF_MX1HqvhKNwlUKh9oQ"
#define CHAT_ID "818675367"
#define BOT_REQUEST_DELAY 1000

UniversalTelegramBot bot(BOT_TOKEN, client);

String scrollMessage = "no message";
unsigned long lastTimeBotRan;
bool ringSent = false;
String scrolling;

String handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    loadStringArray("usersArray" , ChatIDs , 10);
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String ChatUser = "818675367";
    for(int j = 0 ; j < ChatIDs->length() ; j++)
    {
      if(ChatIDs[j] != "")
      {
        if(chat_id == ChatIDs[j])
        {
          ChatUser = ChatIDs[j];
        }
      }
    }
    if (chat_id == ChatUser)
    {
      if (text == "help")
      {
        String welcome = "Hey There ! \n\nTo Set The Time Ex :\n\ntime,10:10:am\n\nTo Set The Date Ex :\n\ndate,2024/10/7 \n\nTo Set The Alarm ON or OFF Ex :\n\nalarm on \n\nTo Set The Alarm Ex :\n\nalarm,10:10:am \n\nTo Set Azan ON or OFF Ex :\n\nazan on \n\nTo Set The Volume (Max Is 30) Ex :\n\nvolume,10 \n\nTo Add New User Ex :\n\nadd,userId \n\nTo Remove a User Ex :\n\nremove,userId \n\nTo List Users :\n\nlist users \n\nTo Ring Just Send ring";
        bot.sendMessage(ChatUser, welcome, "");
      }
      else if(text == "ring")
      {
        ringSent = true;
        bot.sendMessage(ChatUser, "ringing...", "");
      }
      else if (text.startsWith("alarm"))
      {
        if(alarmSetter(text))
        {
          bool Pm = false;
          String formatedTime = formatTime12Hour(AlarmHour , AlarmMins , Pm);
          String checkAlarmSetting = "";
          if(alarmState == "ON")
          {
            if(Pm)
            {
              checkAlarmSetting = "alarm on " + formatedTime + " PM";
            }
            else
            {
              checkAlarmSetting = "alarm on " + formatedTime + " AM";
            }
          }
          else
          {
            checkAlarmSetting = "alarm off";
          }
          bot.sendMessage(ChatUser, checkAlarmSetting, "");
        }
        else
        {
          bot.sendMessage(ChatUser, "Invalid values", "");
        }
      }
      else if (text.startsWith("azan"))
      {
        String azanValidation = "";
        if(azanSetter(text))
        {
          azanValidation = "azan " + prayState;
        }
        else
        {
          azanValidation = "Invalid values";
        }
        bot.sendMessage(ChatUser, azanValidation, "");
      }
      else if (text.startsWith("volume"))
      {
        if (volumeSetter(text))
        {
          String volumeToSend = "volume on " + String(AlarmVolume);
          bot.sendMessage(ChatUser, volumeToSend , "");
        }
        else 
        {
          bot.sendMessage(ChatUser, "Invalid values", "");
        }
      }
      else if (text.startsWith("time"))
      {
        if(timeSetter(text))
        {
          DateTime timeValidation = rtc.now();
          String settingValidation = "";
          bool isPm = false;
          int hourValidation = timeValidation.hour(), minuteValidation = timeValidation.minute();
          String to12Hour = formatTime12Hour(hourValidation , minuteValidation , isPm);
          if(isPm)
          {
            settingValidation = "Time Setted On " + to12Hour + " PM";
          }
          else
          {
            settingValidation = "Time Setted On " + to12Hour + " AM";
          }
          bot.sendMessage(ChatUser, settingValidation, "");
        }
        else
        {
          bot.sendMessage(ChatUser, "Invalid Values", "");
        }
      }
      else if (text.startsWith("date"))
      {
        if(dateSetter(text))
        {
          DateTime dateValidation = rtc.now();
          String settingValidation = "Date Setted On " + String(dateValidation.year()) +"/"+ String(dateValidation.month()) + "/" + String(dateValidation.day());
          bot.sendMessage(ChatUser, settingValidation, "");
        }
        else
        {
          bot.sendMessage(ChatUser, "Invalid Values", "");
        }
      }
      else if (text.startsWith("add"))
      {
        if(addUser(ChatIDs ,  10 , text))
        {
          bot.sendMessage(ChatUser, "User Added", "");
          loadStringArray("usersArray" , ChatIDs , 10);
        }
        else 
        {
          bot.sendMessage(ChatUser, "Invalid Values", "");
        }
      }
      else if (text.startsWith("remove"))
      {
        if(removeUser(ChatIDs ,  10 , text))
        {
          bot.sendMessage(ChatUser, "User Removed", "");
          loadStringArray("usersArray" , ChatIDs , 10);
        }
        else 
        {
          bot.sendMessage(ChatUser, "Invalid Values", "");
        }
      }
      else if (text == "list users")
      {
        String signedUsers = listUsers(ChatIDs , 10);
        if(signedUsers == "")
        {
          bot.sendMessage(ChatUser, "No Users", "");
        }
        else 
        {
          bot.sendMessage(ChatUser, signedUsers, "");
        }
      }
      else
      {
        scrollMessage = text;
        scrolling = text;
        StoreDataString("LastMessge", scrollMessage);
        Serial.println(ReadDataString("LastMessge"));
        bot.sendMessage(ChatUser, "Message received", "");
      }
    }
  }
  return scrollMessage;
}

void handleBotTask(void *parameter)
{
  for (;;)
  {
    if (millis() - lastTimeBotRan > BOT_REQUEST_DELAY)
    {
      if(fetchingAPI == true){
        continue;
      }
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      if (numNewMessages)
      {
        handleNewMessages(numNewMessages);
      }
      lastTimeBotRan = millis();
    }
    esp_task_wdt_reset();                // Reset the watchdog timer
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to yield
  }
}