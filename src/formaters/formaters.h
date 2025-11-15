#ifndef _formaters    // Put these two lines at the top of your file.
#define _formaters    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

String formatTime12Hour(int hour, int minute, bool &isPM);


#endif