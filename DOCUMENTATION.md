# Architecture guide

How the firmware is put together, and the rules to follow when you add to it. For wiring, setup and daily use see the [README](README.md).

## Tasks

Everything runs as FreeRTOS tasks started from one table, `TASKS` in `src/main.cpp`.

| Task | Core | Watchdog | Does |
| :--- | :--- | :--- | :--- |
| Display | 1 | yes | Renders the clock, rotating pages, alarm screen, OTA status. Opens the encoder menu. |
| Time | 1 | yes | Builds the clock and date text once a second. |
| Encoder | 1 | yes | Prints knob movement. |
| AlarmRing | 1 | no | Plays and stops the alarm / azan sound. |
| Bot | 0 | no | Telegram long polling, commands, buttons, firmware updates. |
| Prayer | 0 | no | Fetches prayer times and weather hourly, triggers the azan. |
| Health | 0 | no | Logs memory, restarts only when needed, confirms new firmware. |

Network work is on core 0 so a TLS handshake cannot stall the display on core 1. The Arduino `loop()` only serves the WiFi config page and feeds the watchdog.

## Rules for shared data

Tasks run at the same time, so state that more than one task touches needs care:

- **Saved settings** go through `src/settings`: read with `settings::get()` (a copy), change with `settings::setX()`, which validates, updates memory and writes flash. Never keep your own copy of a setting in a global.
- **Text shared between tasks** (clock, date, scrolling message, weather) is a `SharedString`: `get()` returns a copy, `set()` replaces it, both under a mutex. A plain global `String` is not safe here.
- **The clock chip** is on the I2C bus: use `rtcNow()` / `rtcAdjust()` from `src/rtcClock`, never the RTC object directly.
- **Text given to the LED library** must outlive the call, because it keeps the pointer while it animates. Copy it into a static buffer first (see `hold()` in `src/display/pages.cpp`).
- **Never block a task that feeds the watchdog** for more than a few seconds, and never loop without a delay.

## Adding things

| To add | Do |
| :--- | :--- |
| Telegram command | Handler + one line in `COMMANDS` (`src/telegram/commands.cpp`). Set `ownerOnly` for anything that changes access or firmware. |
| Telegram button | Handler + one line in `CALLBACKS` (`src/telegram/menus.cpp`). |
| Rotating screen | Show function + one line in `PAGES` (`src/display/pages.cpp`). A page can skip itself with `enabled()`. |
| Saved setting | Field in `Settings`, load in `settings::load()`, setter in `settings.cpp`. Keep existing flash keys unchanged. |
| Background task | Task function + one line in `TASKS`. Pick the core and stack size; the Health log shows how much stack is left. |

Put logic that does not need the hardware in its own file and cover it with a test in `test/test_logic/`; those tests run on the PC under AddressSanitizer (`pio test -e native`). Add the folder to `build_src_filter` in the `[env:native]` section.

## Firmware updates

1. The owner sends a `.bin` and then `update <sha256>`.
2. The device streams the file into the inactive firmware slot and hashes it as it goes. A different hash, a stalled download or a failed flash write aborts the update.
3. The device restarts into the new slot, which stays *pending* for two minutes. `healthTask` confirms it after that. A crash or power cut before then makes the bootloader return to the previous slot.
4. The owner gets a Telegram message with the outcome.

The checksum guards against a wrong or corrupted file. It does not stop someone who controls the owner's Telegram account, since they could send a matching hash; that would need signed firmware.

## Settings storage

All values live in the `store` flash namespace under the keys `alarmHour`, `alarmMins`, `almState`, `prayState`, `AlarmVolume`, `AlarmMusic`, `br` and `LastMessge` (these names date from the first version and are kept so updated devices keep their settings). Prayer times and extra Telegram users are stored as string arrays in the same namespace. The bot token and owner chat ID are stored by the WiFi portal library. Health data (last restart reason, last update result) is in the `health` namespace.
