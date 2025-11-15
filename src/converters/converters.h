#ifndef _converters    // Put these two lines at the top of your file.
#define _converters    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

int nat(int number);
String longToString(long number);
void convertTo24Hour(int hour12, int minute, const String &period, int &hour24, int &minute24);


#endif
