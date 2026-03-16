#include <Arduino.h>
#include <AsyncTelegram2.h>
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

#define BOT_TOKEN "8638569625:AAF81cLSgBvPPXOc6Yphx2eWlByATaUZgsg"
#define CHAT_ID "818675367"
#define BOT_REQUEST_DELAY 1000

AsyncTelegram2 bot(client);

String scrollMessage = "no message";
unsigned long lastTimeBotRan;
bool ringSent = false;
String scrolling;

void handleNewMessage(TBMessage &msg)
{
    loadStringArray("usersArray" , ChatIDs , 10);
    String chat_id = String(msg.chatId);
    String text = msg.text;
    Serial.println(text);
    
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
        bot.sendMessage(msg, welcome);
      }
      else if(text == "ring")
      {
        ringSent = true;
        bot.sendMessage(msg, "ringing...");
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
          bot.sendMessage(msg, checkAlarmSetting);
        }
        else
        {
          bot.sendMessage(msg, "Invalid values");
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
        bot.sendMessage(msg, azanValidation);
      }
      else if (text.startsWith("volume"))
      {
        if (volumeSetter(text))
        {
          String volumeToSend = "volume on " + String(AlarmVolume);
          bot.sendMessage(msg, volumeToSend);
        }
        else 
        {
          bot.sendMessage(msg, "Invalid values");
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
          bot.sendMessage(msg, settingValidation);
        }
        else
        {
          bot.sendMessage(msg, "Invalid Values");
        }
      }
      else if (text.startsWith("date"))
      {
        if(dateSetter(text))
        {
          DateTime dateValidation = rtc.now();
          String settingValidation = "Date Setted On " + String(dateValidation.year()) +"/"+ String(dateValidation.month()) + "/" + String(dateValidation.day());
          bot.sendMessage(msg, settingValidation);
        }
        else
        {
          bot.sendMessage(msg, "Invalid Values");
        }
      }
      else if (text.startsWith("add"))
      {
        if(addUser(ChatIDs ,  10 , text))
        {
          bot.sendMessage(msg, "User Added");
          loadStringArray("usersArray" , ChatIDs , 10);
        }
        else 
        {
          bot.sendMessage(msg, "Invalid Values");
        }
      }
      else if (text.startsWith("remove"))
      {
        if(removeUser(ChatIDs ,  10 , text))
        {
          bot.sendMessage(msg, "User Removed");
          loadStringArray("usersArray" , ChatIDs , 10);
        }
        else 
        {
          bot.sendMessage(msg, "Invalid Values");
        }
      }
      else if (text == "list users")
      {
        String signedUsers = listUsers(ChatIDs , 10);
        if(signedUsers == "")
        {
          bot.sendMessage(msg, "No Users");
        }
        else 
        {
          bot.sendMessage(msg, signedUsers);
        }
      }
      else
      {
        scrollMessage = text;
        scrolling = text;
        StoreDataString("LastMessge", scrollMessage);
        Serial.println(ReadDataString("LastMessge"));
        bot.sendMessage(msg, "Message received");
      }
    }
}

void handleBotTask(void *parameter)
{
  bot.setUpdateTime(BOT_REQUEST_DELAY);
  bot.setTelegramToken(BOT_TOKEN);
  
  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  Serial.println("Telegram bot starting...");
  if (bot.begin()) {
    Serial.println("Telegram bot started");
  } else {
    Serial.println("Telegram bot failed to start");
  }

  for (;;)
  {
    if(fetchingAPI == false){
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
