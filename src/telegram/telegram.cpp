#include <Arduino.h>
#include <AsyncTelegram2.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <wifi/wifi.h>
#include <flashUtils/flashUtils.h>
#include <setters/setters.h>
#include <time/time.h>
#include <formaters/formaters.h>
#include <parsers/parsers.h>
#include <converters/converters.h>
#include <encoder/encoder.h>
#include <usersManagement/usersManagement.h>
#include <prayerTime/prayerTime.h>
#include <weather/weather.h>
#include <HTTPUpdate.h>
#include <display/display.h>

#define BOT_TOKEN "8638569625:AAF81cLSgBvPPXOc6Yphx2eWlByATaUZgsg"
#define CHAT_ID "818675367"
#define BOT_REQUEST_DELAY 1000

// Callback constants for button identification
#define CB_MAIN          "main"
#define CB_EDIT_MSG      "edit_msg"
#define CB_ALARM_MENU    "alarm_menu"
#define CB_TIME_MENU     "time_menu"
#define CB_VOL_MENU      "vol_menu"
#define CB_AZAN_TOGGLE   "azan_toggle"
#define CB_REFRESH       "refresh"
#define CB_WEATHER_REF   "weather_ref"
#define CB_RING          "ring_bot"
#define CB_ALARM_TOG_M   "alm_tog_m"

#define CB_ALARM_H_UP    "alm_h_u"
#define CB_ALARM_H_DN    "alm_h_d"
#define CB_ALARM_M_UP    "alm_m_u"
#define CB_ALARM_M_DN    "alm_m_d"
#define CB_ALARM_P_TOG   "alm_p_t"
#define CB_ALARM_SAVE    "alm_save"
#define CB_ALARM_TOGGLE  "alm_toggle"

AsyncTelegram2 bot(client);

String scrollMessage = "no message";
unsigned long lastTimeBotRan;
bool ringSent = false;
String scrolling;

// Draft variables for interactive editing
int draftHour = 12;
int draftMin = 0;
bool draftIsPm = false;
bool isEditingMsg = false;

void showMainMenu(TBMessage &msg, bool edit = false) {
    InlineKeyboard mainKbd(BUFFER_MEDIUM);
    mainKbd.addButton("📝 Msg", CB_EDIT_MSG, KeyboardButtonQuery);
    mainKbd.addButton("🔔 Ring", CB_RING, KeyboardButtonQuery);
    mainKbd.addRow();
    
    mainKbd.addButton("🕒 Time", CB_TIME_MENU, KeyboardButtonQuery);
    mainKbd.addButton("🔊 Vol", CB_VOL_MENU, KeyboardButtonQuery);
    mainKbd.addRow();

    mainKbd.addButton("⏰ Set", CB_ALARM_MENU, KeyboardButtonQuery);
    String almToggleLabel = (alarmState == "ON") ? "⏰ ON" : "⏰ OFF";
    mainKbd.addButton(almToggleLabel.c_str(), CB_ALARM_TOG_M, KeyboardButtonQuery);
    mainKbd.addRow();

    String azanLabel = (prayState == "ON") ? "🕋 Azan: ON" : "🕋 Azan: OFF";
    mainKbd.addButton(azanLabel.c_str(), CB_AZAN_TOGGLE, KeyboardButtonQuery);
    mainKbd.addRow();
    
    mainKbd.addButton("🌤 Weather", CB_WEATHER_REF, KeyboardButtonQuery);
    mainKbd.addButton("🔄 Refresh", CB_REFRESH, KeyboardButtonQuery);

    String status = "🏠 *LED Matrix Dashboard*\n";
    status += "---------------------------\n";
    status += "💬 *Msg:* \"" + scrolling + "\"\n";
    
    bool pm = false;
    String almTime = formatTime12Hour(AlarmHour, AlarmMins, pm);
    status += "⏰ *Alarm:* " + almTime + (pm ? " PM" : " AM") + " (" + alarmState + ")\n";
    status += "🕋 *Azan:* " + prayState + "\n";
    status += "🔊 *Volume:* " + String(AlarmVolume) + "\n";
    status += "🌤 *Weather:* " + weatherTemp;

    if (edit) {
        bot.editMessage(msg, status.c_str(), mainKbd);
    } else {
        bot.sendMessage(msg, status.c_str(), mainKbd);
    }
}

void showAlarmMenu(TBMessage &msg) {
    InlineKeyboard almKbd;
    almKbd.addButton("H +", CB_ALARM_H_UP, KeyboardButtonQuery);
    almKbd.addButton("M +", CB_ALARM_M_UP, KeyboardButtonQuery);
    almKbd.addRow();
    
    char timeStr[20];
    sprintf(timeStr, "%02d : %02d %s", draftHour, draftMin, draftIsPm ? "PM" : "AM");
    almKbd.addButton(timeStr, "none", KeyboardButtonQuery);
    almKbd.addRow();

    almKbd.addButton("H -", CB_ALARM_H_DN, KeyboardButtonQuery);
    almKbd.addButton("M -", CB_ALARM_M_DN, KeyboardButtonQuery);
    almKbd.addRow();
    
    almKbd.addButton(draftIsPm ? "Switch to AM" : "Switch to PM", CB_ALARM_P_TOG, KeyboardButtonQuery);
    almKbd.addRow();
    
    String toggleLabel = (alarmState == "ON") ? "Turn OFF" : "Turn ON";
    almKbd.addButton(toggleLabel.c_str(), CB_ALARM_TOGGLE, KeyboardButtonQuery);
    almKbd.addRow();

    almKbd.addButton("✅ SAVE", CB_ALARM_SAVE, KeyboardButtonQuery);
    almKbd.addButton("❌ Back", CB_MAIN, KeyboardButtonQuery);

    bot.editMessage(msg, "⏰ *Set Alarm Time*", almKbd);
}

void showVolMenu(TBMessage &msg) {
    InlineKeyboard volKbd;
    volKbd.addButton("5", "vol_5", KeyboardButtonQuery);
    volKbd.addButton("10", "vol_10", KeyboardButtonQuery);
    volKbd.addButton("15", "vol_15", KeyboardButtonQuery);
    volKbd.addRow();
    volKbd.addButton("20", "vol_20", KeyboardButtonQuery);
    volKbd.addButton("25", "vol_25", KeyboardButtonQuery);
    volKbd.addButton("30", "vol_30", KeyboardButtonQuery);
    volKbd.addRow();
    volKbd.addButton("❌ Back", CB_MAIN, KeyboardButtonQuery);

    bot.editMessage(msg, "🔊 *Set Volume*", volKbd);
}

#define CB_TIME_H_UP    "time_h_u"
#define CB_TIME_H_DN    "time_h_d"
#define CB_TIME_M_UP    "time_m_u"
#define CB_TIME_M_DN    "time_m_d"
#define CB_TIME_P_TOG   "time_p_t"
#define CB_TIME_SAVE    "time_save"

void showTimeMenu(TBMessage &msg) {
    InlineKeyboard timeKbd;
    timeKbd.addButton("H +", CB_TIME_H_UP, KeyboardButtonQuery);
    timeKbd.addButton("M +", CB_TIME_M_UP, KeyboardButtonQuery);
    timeKbd.addRow();
    
    char timeStr[20];
    sprintf(timeStr, "%02d : %02d %s", draftHour, draftMin, draftIsPm ? "PM" : "AM");
    timeKbd.addButton(timeStr, "none", KeyboardButtonQuery);
    timeKbd.addRow();

    timeKbd.addButton("H -", CB_TIME_H_DN, KeyboardButtonQuery);
    timeKbd.addButton("M -", CB_TIME_M_DN, KeyboardButtonQuery);
    timeKbd.addRow();
    
    timeKbd.addButton(draftIsPm ? "Switch to AM" : "Switch to PM", CB_TIME_P_TOG, KeyboardButtonQuery);
    timeKbd.addRow();

    timeKbd.addButton("✅ UPDATE RTC", CB_TIME_SAVE, KeyboardButtonQuery);
    timeKbd.addRow();
    timeKbd.addButton("❌ Back", CB_MAIN, KeyboardButtonQuery);

    bot.editMessage(msg, "🕒 *Set System Time*", timeKbd);
}

void handleNewMessage(TBMessage &msg)
{
    loadStringArray("usersArray" , ChatIDs , 10);
    String chat_id = String(msg.chatId);
    String text = msg.text;
    
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

    if (chat_id != ChatUser) return;

    // Handle button clicks
    if (msg.messageType == MessageQuery) {
        String data = msg.callbackQueryData;
        
        if (data == CB_REFRESH || data == CB_MAIN) {
            fetchWeather();
            showMainMenu(msg, true);
        }
        else if (data == CB_AZAN_TOGGLE) {
            azanSetter(prayState == "ON" ? "azan off" : "azan on");
            showMainMenu(msg, true);
        }
        else if (data == CB_ALARM_TOG_M) {
            alarmSetter(alarmState == "ON" ? "ALARM OFF" : "ALARM ON");
            showMainMenu(msg, true);
        }
        else if (data == CB_WEATHER_REF) {
            fetchWeather();
            showMainMenu(msg, true);
        }
        else if (data == CB_RING) {
            ringSent = true;
            bot.endQuery(msg, "Ringing...");
            return;
        }
        else if (data == CB_EDIT_MSG) {
            isEditingMsg = true;
            bot.sendMessage(msg, "📝 Send me the new message to scroll:");
        }
        else if (data == CB_ALARM_MENU) {
            // Load current alarm into draft
            draftIsPm = false;
            String alm12 = formatTime12Hour(AlarmHour, AlarmMins, draftIsPm);
            parseTime(alm12, draftHour, draftMin, data); // reuse data as temp
            showAlarmMenu(msg);
        }
        else if (data == CB_ALARM_H_UP) {
            draftHour = (draftHour % 12) + 1;
            showAlarmMenu(msg);
        }
        else if (data == CB_ALARM_H_DN) {
            draftHour = (draftHour == 1) ? 12 : draftHour - 1;
            showAlarmMenu(msg);
        }
        else if (data == CB_ALARM_M_UP) {
            draftMin = (draftMin + 5) % 60;
            showAlarmMenu(msg);
        }
        else if (data == CB_ALARM_M_DN) {
            draftMin = (draftMin < 5) ? 55 : draftMin - 5;
            showAlarmMenu(msg);
        }
        else if (data == CB_ALARM_P_TOG) {
            draftIsPm = !draftIsPm;
            showAlarmMenu(msg);
        }
        else if (data == CB_ALARM_TOGGLE) {
            alarmSetter(alarmState == "ON" ? "ALARM OFF" : "ALARM ON");
            showAlarmMenu(msg);
        }
        else if (data == CB_VOL_MENU) {
            showVolMenu(msg);
        }
        else if (data == CB_TIME_MENU) {
            DateTime now = rtc.now();
            String dummy;
            String t12 = formatTime12Hour(now.hour(), now.minute(), draftIsPm);
            parseTime(t12, draftHour, draftMin, dummy);
            showTimeMenu(msg);
        }
        else if (data == CB_TIME_H_UP) {
            draftHour = (draftHour % 12) + 1;
            showTimeMenu(msg);
        }
        else if (data == CB_TIME_H_DN) {
            draftHour = (draftHour == 1) ? 12 : draftHour - 1;
            showTimeMenu(msg);
        }
        else if (data == CB_TIME_M_UP) {
            draftMin = (draftMin + 1) % 60;
            showTimeMenu(msg);
        }
        else if (data == CB_TIME_M_DN) {
            draftMin = (draftMin == 0) ? 59 : draftMin - 1;
            showTimeMenu(msg);
        }
        else if (data == CB_TIME_P_TOG) {
            draftIsPm = !draftIsPm;
            showTimeMenu(msg);
        }
        else if (data == CB_TIME_SAVE) {
            int h24, m24;
            convertTo24Hour(draftHour, draftMin, draftIsPm ? "PM" : "AM", h24, m24);
            DateTime now = rtc.now();
            rtc.adjust(DateTime(now.year(), now.month(), now.day(), h24, m24, 0));
            bot.sendMessage(msg, "✅ System time updated!");
            showMainMenu(msg);
        }
        else if (data.startsWith("vol_")) {
            String volStr = data.substring(4);
            volumeSetter("volume " + volStr);
            showMainMenu(msg, true);
        }
        else if (data == CB_ALARM_SAVE) {
            int h24, m24;
            convertTo24Hour(draftHour, draftMin, draftIsPm ? "PM" : "AM", h24, m24);
            StoreData("alarmHour", h24);
            StoreData("alarmMins", m24);
            AlarmHour = h24;
            AlarmMins = m24;
            bot.sendMessage(msg, "✅ Alarm saved!");
            showMainMenu(msg);
        }
        
        bot.endQuery(msg, "Updated");
        return;
    }

    // Handle normal text messages
    if (isEditingMsg) {
        scrollMessage = text;
        scrolling = text;
        StoreDataString("LastMessge", scrollMessage);
        isEditingMsg = false;
        bot.sendMessage(msg, "✅ Message updated!");
        showMainMenu(msg);
        return;
    }

    if (text == "/start" || text == "help") {
        showMainMenu(msg);
    }
    else if (text.startsWith("update ")) {
        String url = text.substring(7);
        url.trim();
        bot.sendMessage(msg, "Starting OTA update from:\n" + url + "\n\nPlease wait...");
        
        myDisplay.displayClear();
        myDisplay.displayText("UPDATING...", PA_CENTER, 50, 0, PA_PRINT, PA_PRINT);
        myDisplay.displayAnimate();
        
        // Disable watchdog so ESP32 doesn't restart during download
        esp_task_wdt_delete(NULL);
        
        httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        t_httpUpdate_return ret = httpUpdate.update(client, url);
        
        switch (ret) {
            case HTTP_UPDATE_FAILED: {
                String err = "Update failed.\nError (" + String(httpUpdate.getLastError()) + "): " + httpUpdate.getLastErrorString();
                bot.sendMessage(msg, err);
                Serial.println(err);
                // Re-enable watchdog
                esp_task_wdt_add(NULL);
                myDisplay.displayClear(); // clear "UPDATING..."
                break;
            }
            case HTTP_UPDATE_NO_UPDATES:
                bot.sendMessage(msg, "No updates available.");
                esp_task_wdt_add(NULL);
                myDisplay.displayClear(); // clear "UPDATING..."
                break;
            case HTTP_UPDATE_OK:
                bot.sendMessage(msg, "Update successful! Rebooting...");
                Serial.println("Update OK. Rebooting...");
                delay(1000);
                ESP.restart();
                break;
        }
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
