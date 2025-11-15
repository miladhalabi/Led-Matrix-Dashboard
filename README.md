
# LED Matrix Dashboard Documentation

This document provides a comprehensive overview of the LED Matrix Dashboard project, including its features, hardware requirements, software dependencies, and usage instructions.

## 1. Project Overview

The LED Matrix Dashboard is a feature-rich, network-enabled display system built around an ESP32 microcontroller. It drives a MAX7219-based LED matrix to show time, date, custom messages, and animations. The project integrates with Telegram for remote control, fetches prayer times from an online API, and provides a web interface for initial WiFi configuration. It also features a rotary encoder for manual interaction and a DFPlayer Mini for audio alarms.

## 2. Features

*   **Time and Date Display:** Shows the current time and date, automatically synced via NTP.
*   **Custom Message Scrolling:** Display custom messages sent via a Telegram bot.
*   **Custom Animations:** Supports custom animations on the LED matrix.
*   **Telegram Bot Control:** A Telegram bot allows for remote control of the device, including setting the time, date, alarm, volume, and custom messages.
*   **Prayer Times:** Fetches daily prayer times from an online API and can play an Azan notification.
*   **Alarm Clock:** A configurable alarm with a custom volume and sound, which can be set via the rotary encoder or Telegram.
*   **Rotary Encoder Interface:** A rotary encoder with a push-button allows for manual navigation of a menu system to configure settings directly on the device.
*   **WiFi Configuration:** An initial access point and web portal (IotWebConf) allows for easy configuration of WiFi credentials.
*   **Persistent Storage:** Settings like brightness, alarm time, volume, and Telegram messages are saved to flash memory and persist across reboots.
*   **Over-the-Air (OTA) Updates:** The framework supports OTA updates, although not explicitly implemented in the current code.
*   **Multi-tasking:** The project uses FreeRTOS tasks to handle the display, encoder, Telegram bot, timekeeping, and other functions concurrently.
*   **Watchdog Timer:** A watchdog timer is implemented to automatically restart the device if it becomes unresponsive.

## 3. Hardware Requirements & Connections

*   **ESP32 Dev Kit:** The core of the project.
*   **MAX7219 LED Matrix Display:** A 4-module (8x32) display.
    *   `CS_PIN`: GPIO 5
*   **Rotary Encoder with Push-button:** For manual control.
    *   `ENCODER_PIN_A`: GPIO 33
    *   `ENCODER_PIN_B`: GPIO 25
    *   `PRESS_PIN`: GPIO 32
*   **DS3231 Real-Time Clock (RTC) Module:** For accurate timekeeping, connected via I2C.
    *   SDA: Default ESP32 I2C SDA pin
    *   SCL: Default ESP32 I2C SCL pin
*   **DFPlayer Mini MP3 Player Module:** For playing alarm sounds.
    *   RX: Connected to a TX pin on the ESP32 (via `Serial2`)
    *   TX: Connected to an RX pin on the ESP32 (via `Serial2`)
*   **General Purpose LED:**
    *   `LED_PIN`: GPIO 2

## 4. Software Dependencies

The project is built using PlatformIO and the Arduino framework. The following libraries are required and are listed in the `platformio.ini` file:

*   **MD_MAX72XX:** For controlling the MAX7219 LED matrix.
*   **MD_Parola:** A library for creating text effects and animations on the LED matrix.
*   **ArduinoJson:** For parsing JSON data from the prayer times API and Telegram.
*   **Universal-Arduino-Telegram-Bot:** For interacting with the Telegram Bot API.
*   **IotWebConf:** For creating the WiFi configuration portal.
*   **NTPClient:** For synchronizing the time with an NTP server.
*   **ESP32Encoder:** For reading the rotary encoder.
*   **RTClib:** For interfacing with the DS3231 RTC module.
*   **DFRobotDFPlayerMini:** For controlling the DFPlayer Mini MP3 player.
*   **espsoftwareserial:** Software serial library for ESP32.

## 5. Configuration

*   **WiFi Credentials:** Configured via the IotWebConf portal on first boot. The device creates a WiFi access point named "testThing" with the password "m12345678". Connect to this network and navigate to 192.168.4.1 in a web browser to configure the device to connect to your local WiFi network.
*   **Telegram Bot Token:** The Telegram bot token is hardcoded in `src/telegram/telegram.cpp`. You will need to create your own bot using the BotFather on Telegram and replace the placeholder token.
*   **Chat ID:** The primary chat ID is also hardcoded in `src/telegram/telegram.cpp`. This should be your personal Telegram chat ID to receive messages and control the bot.

## 6. Code Structure

The project is organized into several modules, each with a specific responsibility.

### `main.cpp`
The main entry point of the application. It initializes all the hardware, libraries, and FreeRTOS tasks.

### Modules

*   **`converters`:** Utility functions for converting data, such as 12-hour to 24-hour time format.
*   **`display`:** Manages the LED matrix display, including scrolling text and animations.
*   **`encoder`:** Handles the rotary encoder input and the menu system.
*   **`flashUtils`:** Provides functions for reading and writing data to the ESP32's flash memory using the `Preferences` library.
*   **`formaters`:** Functions for formatting data, such as creating a 12-hour time string.
*   **`parsers`:** Functions for parsing strings, such as extracting data from Telegram messages or parsing custom animation data.
*   **`prayerTime`:** Fetches and manages prayer times from the Al-Adhan API.
*   **`setters`:** Functions that handle setting new values for various parameters, often called from the Telegram message handler.
*   **`telegram`:** Manages the Telegram bot, including receiving and processing messages.
*   **`time`:** Manages the DS3231 RTC, keeps track of the current time and date, and handles the alarm logic.
*   **`usersManagement`:** Functions for adding, removing, and listing authorized Telegram users.
*   **`utils`:** Miscellaneous utility functions, such as a watchdog monitor and a function to detect long presses on the encoder button.
*   **`wifi`:** Initializes and manages the WiFi connection using the IotWebConf library.

## 7. Building and Uploading

1.  Install [PlatformIO](https://platformio.org/).
2.  Open the project in Visual Studio Code with the PlatformIO extension.
3.  Connect the ESP32 to your computer.
4.  Build and upload the project using the PlatformIO toolbar.

## 8. Usage

### Initial Setup
1.  Power on the device.
2.  On your phone or computer, connect to the "testThing" WiFi network with the password "m12345678".
3.  Open a web browser and go to `192.168.4.1`.
4.  Configure the device to connect to your local WiFi network.
5.  The device will restart and connect to your WiFi.

### Telegram Control
You can control the device by sending messages to your Telegram bot. The available commands are:

*   `help`: Displays a list of available commands.
*   `time,HH:MM:AM/PM`: Sets the time. Example: `time,10:30:pm`
*   `date,YYYY/MM/DD`: Sets the date. Example: `date,2025/08/22`
*   `alarm on`/`alarm off`: Enables or disables the alarm.
*   `alarm,HH:MM:AM/PM`: Sets the alarm time. Example: `alarm,07:00:am`
*   `azan on`/`azan off`: Enables or disables the Azan notification.
*   `volume,VV`: Sets the alarm volume (0-30). Example: `volume,25`
*   `add,USER_ID`: Adds a new authorized user.
*   `remove,USER_ID`: Removes an authorized user.
*   `list users`: Lists all authorized users.
*   `ring`: Triggers the alarm sound.
*   Any other text will be displayed as a scrolling message on the LED matrix.

### Manual Control (Rotary Encoder)
*   **Rotate:** Navigate through the menu options.
*   **Short Press:** Not implemented in the main menu, but used within sub-menus.
*   **Long Press:** Enter the menu or select an option.

The menu options include:
*   Set Time
*   Set Alarm
*   Set Brightness
*   Set Alarm Status (On/Off)
*   Set Alarm Volume
*   Set Alarm Music
*   Set Prayer Time Status (On/Off)
*   Exit
