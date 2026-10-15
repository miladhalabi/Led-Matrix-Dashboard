#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <converters/converters.h>
#include <formaters/formaters.h>
#include <flashUtils/flashUtils.h>
#include <settings/settings.h>
#include <parsers/parsers.h>
#include <utils/utils.h>
#include <wifi/wifi.h>
#include <telegram/telegram.h>
#include <encoder/encoder.h>
#include <display/display.h>
#include <time/time.h>
#include <prayerTime/prayerTime.h>
#include <rtcClock/rtcClock.h>
#include <health/health.h>

bool ledState = LOW;
String heartString = "heart";

#define WATCHDOG_TIMEOUT 10
#define LED_PIN 2
#define PRESS_PIN 32
#define ENCODER_PIN_A 33
#define ENCODER_PIN_B 25

// Every background task. To add a service: write its task function and add a line here.
struct TaskSpec
{
  TaskFunction_t function;
  const char *name;
  uint32_t stackBytes;
  BaseType_t core; // 0 = with the WiFi stack, 1 = with the Arduino loop
  bool watchdog;   // must call esp_task_wdt_reset() within WATCHDOG_TIMEOUT seconds
};

// Network work (Telegram TLS, HTTP fetches) runs on core 0 so it cannot stall the display on core 1.
// Stack sizes: the health task logs each task's lowest free stack, so trim them from that log.
static const TaskSpec TASKS[] = {
    {handleEncoderTask, "Encoder", 4096, 1, true},
    {handleDisplayTask, "Display", 20000, 1, true},
    {handleTimeTask, "Time", 8192, 1, true},
    {alarmRing, "AlarmRing", 4096, 1, false},
    {handleBotTask, "Bot", 20000, 0, false},
    {isPrayTime, "Prayer", 8192, 0, false},
    {healthTask, "Health", 4096, 0, false},
};

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);
  Serial.println("===================UPDATED===================");
  healthInit();
  prepareDeviceIdentity();

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(PRESS_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, ledState);

  if (!rtcBegin())
  {
    Serial.println("RTC module is NOT found");
  }

  encoder.attachSingleEdge(ENCODER_PIN_A, ENCODER_PIN_B);
  encoder.clearCount();

  setupTelegramConfig();
  iotWebConf.init();

  server.on("/", handleRoot);
  server.on("/config", []
            { iotWebConf.handleConfig(); });
  server.onNotFound([]
                    { iotWebConf.handleNotFound(); });

  Serial.println("Starting up...");
  Serial.println(WiFi.localIP());
  settings::load();
  const Settings cfg = settings::get();

  if (player.begin(Serial2, true))
  {
    Serial.println("OK");
    player.volume(cfg.volume);
  }
  else
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }


  myDisplay.begin();
  myDisplay.setIntensity(cfg.brightness);
  myDisplay.displayClear();
  myDisplay.addChar(1, customChar);
  myDisplay.addChar(2 , customChar1);
  myDisplay.addChar(3, sunGlyph);
  myDisplay.addChar(4, cloudGlyph);
  myDisplay.addChar(5, rainGlyph);
  myDisplay.addChar(6, moonGlyph);

  scrolling.set(settings::lastMessage());

  // loadStringArray("prayTimeArray", prayTimeArray, 5);

#ifdef ESP32
  client.setInsecure();
#endif
#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");
  client.setInsecure();
#elif defined(ESP32)
  configTime(0, 0, "pool.ntp.org");
#endif

  esp_task_wdt_init(WATCHDOG_TIMEOUT, true); // Enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                    // Add the main loop task to the watchdog

  for (const TaskSpec &task : TASKS)
  {
    TaskHandle_t handle = NULL;
    xTaskCreatePinnedToCore(task.function, task.name, task.stackBytes, NULL, 1, &handle, task.core);
    if (handle == NULL)
    {
      Serial.printf("Could not start task %s\n", task.name);
      continue;
    }
    if (task.watchdog)
      esp_task_wdt_add(handle);
    healthWatchTask(task.name, handle);
  }
}

void loop()
{
  iotWebConf.doLoop();
  esp_task_wdt_reset(); // Reset the watchdog timer in the main loop
}
