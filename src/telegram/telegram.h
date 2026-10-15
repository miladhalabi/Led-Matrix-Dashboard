#ifndef _telegram    // Put these two lines at the top of your file.
#define _telegram    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <AsyncTelegram2.h>
#include <shared/shared_string.h>

extern bool ringSent;
extern SharedString scrolling;
extern volatile bool isOTAUpdating;
extern SharedString otaProgressText;

void handleNewMessage(TBMessage &msg);
void handleBotTask(void *parameter);


#endif