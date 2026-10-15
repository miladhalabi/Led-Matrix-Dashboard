#ifndef _flashUtils    // Put these two lines at the top of your file.
#define _flashUtils    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <Preferences.h>

//extern Preferences preferences;

void storeStringArray(String keyPrefix, String arr[], int size);
void loadStringArray(String keyPrefix, String arr[], int size);


#endif
