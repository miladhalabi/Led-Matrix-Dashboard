#ifndef _telegram    // Put these two lines at the top of your file.
#define _telegram    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

extern bool ringSent;
extern String scrolling;

String handleNewMessages(int numNewMessages);
void handleBotTask(void *parameter);


#endif