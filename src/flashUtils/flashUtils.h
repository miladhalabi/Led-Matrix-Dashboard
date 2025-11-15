#ifndef _flashUtils    // Put these two lines at the top of your file.
#define _flashUtils    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <Preferences.h>

//extern Preferences preferences;

void StoreData(const char *key, const int val);
int ReadData(const char *val);
void StoreDataString(const char *key, const String val);
String ReadDataString(const char *val);
void writeToFlash(const char *key, int valueToStore);
int readFromFlash(const char *key);
void storeStringArray(String keyPrefix, String arr[], int size);
void loadStringArray(String keyPrefix, String arr[], int size);


#endif
