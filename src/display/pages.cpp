#include <Arduino.h>
#include <MD_Parola.h>
#include <time/time.h>
#include <telegram/telegram.h>
#include <parsers/parsers.h>
#include <wifi/wifi.h>
#include "display.h"
#include "pages.h"

/**
 * The pages the matrix rotates through (the clock is shown in between, by the display task).
 * To add one: write a show function that starts an animation on myDisplay, and add a line to PAGES.
 * `enabled` lets a page skip itself, e.g. a prayer-times page while the feature is off.
 */
namespace
{
  const uint16_t PAUSE_TIME = 1000; // in milliseconds

  // Parola keeps the pointer it is given for as long as it animates, so the text it shows
  // lives in buffers of our own (a temporary String would be gone by then).
  char messageText[512];
  char dateText[128];
  char setupText[96];

  const char *hold(char *buffer, size_t size, const String &text)
  {
    strlcpy(buffer, text.c_str(), size);
    return buffer;
  }

const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] = // gobbling pacman animation
    {0x00,0x81,0xc3,0xe7,0xff,0x7e,0x7e,0x3c,0x00,0x42,0xe7,0xe7,0xff,0xff,0x7e,0x3c,0x24,0x66,0xe7,0xff,0xff,0xff,0x7e,0x3c,0x3c,0x7e,0xff,0xff,0xff,0xff,0x7e,0x3c,0x24,0x66,0xe7,0xff,0xff,0xff,0x7e,0x3c,0x00,0x42,0xe7,0xe7,0xff,0xff,0x7e,0x3c,};
const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] = // ghost pursued by a pacman
    {0x00,0x81,0xc3,0xe7,0xff,0x7e,0x7e,0x3c,0x00,0x00,0x00,0xfe,0x7b,0xf3,0x7f,0xfb,0x73,0xfe,0x00,0x42,0xe7,0xe7,0xff,0xff,0x7e,0x3c,0x00,0x00,0x00,0xfe,0x7b,0xf3,0x7f,0xfb,0x73,0xfe,0x24,0x66,0xe7,0xff,0xff,0xff,0x7e,0x3c,0x00,0x00,0x00,0xfe,0x7b,0xf3,0x7f,0xfb,0x73,0xfe,0x3c,0x7e,0xff,0xff,0xff,0xff,0x7e,0x3c,0x00,0x00,0x00,0xfe,0x7b,0xf3,0x7f,0xfb,0x73,0xfe,0x24,0x66,0xe7,0xff,0xff,0xff,0x7e,0x3c,0x00,0x00,0x00,0xfe,0x7b,0xf3,0x7f,0xfb,0x73,0xfe,0x00,0x42,0xe7,0xe7,0xff,0xff,0x7e,0x3c,0x00,0x00,0x00,0xfe,0x7b,0xf3,0x7f,0xfb,0x73,0xfe,};
const uint8_t F_HEART = 5;
const uint8_t W_HEART = 9;
const uint8_t PROGMEM heart[F_HEART * W_HEART] =
    {0x0e,0x11,0x21,0x42,0x84,0x42,0x21,0x11,0x0e,0x0e,0x1f,0x33,0x66,0xcc,0x66,0x33,0x1f,0x0e,0x0e,0x1f,0x3f,0x7e,0xfc,0x7e,0x3f,0x1f,0x0e,0x0e,0x1f,0x33,0x66,0xcc,0x66,0x33,0x1f,0x0e,0x0e,0x11,0x21,0x42,0x84,0x42,0x21,0x11,0x0e,};

  uint8_t *customAnim = nullptr;
  uint8_t customFrames = 6;
  uint8_t customWidth = 8;

  struct Page
  {
    const char *name;
    bool (*enabled)();
    void (*show)();
  };

  bool always() { return true; }

  // The user's text, or one of the built-in animations when the text is "pacman", "heart" or "anim...".
  void showMessagePage()
  {
    const String message = scrolling.get();
    if (message == "pacman")
    {
      myDisplay.displayText("", PA_CENTER, 50, PAUSE_TIME, PA_SPRITE, PA_SPRITE);
      myDisplay.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);
    }
    else if (message == "heart")
    {
      myDisplay.displayText("", PA_CENTER, 50, PAUSE_TIME, PA_SPRITE);
      myDisplay.setSpriteData(heart, W_HEART, F_HEART, heart, W_HEART, F_HEART);
    }
    else if (message.startsWith("anim") && parseSpriteData(message, customAnim, customFrames, customWidth))
    {
      myDisplay.displayText("", PA_CENTER, 50, PAUSE_TIME, PA_SPRITE, PA_SPRITE);
      myDisplay.setSpriteData(customAnim, customWidth, customFrames, customAnim, customWidth, customFrames);
    }
    else
    {
      myDisplay.displayScroll(hold(messageText, sizeof(messageText), message), PA_CENTER, PA_SCROLL_LEFT, 50);
    }
  }

  // "2026/10/15  24° <weather glyph> <alarm / azan icons>"
  void showDatePage()
  {
    myDisplay.displayScroll(hold(dateText, sizeof(dateText), dateNow.get()), PA_CENTER, PA_SCROLL_LEFT, 50);
  }

  // Until WiFi is configured the matrix tells you how to reach the setup page.
  bool needsSetup() { return waitingForFirstSetup(); }

  void showSetupPage()
  {
    const String text = String("Setup: join WiFi ") + setupWifiName() + "  password " + setupWifiPassword() + "  then open 192.168.4.1";
    myDisplay.displayScroll(hold(setupText, sizeof(setupText), text), PA_CENTER, PA_SCROLL_LEFT, 50);
  }

  const Page PAGES[] = {
      {"setup", needsSetup, showSetupPage},
      {"message", always, showMessagePage},
      {"date", always, showDatePage},
  };

  size_t nextPage = 0;
}

void showNextPage()
{
  const size_t count = sizeof(PAGES) / sizeof(PAGES[0]);
  for (size_t tried = 0; tried < count; tried++)
  {
    const Page &page = PAGES[nextPage];
    nextPage = (nextPage + 1) % count;
    if (page.enabled())
    {
      page.show();
      return;
    }
  }
}
