#ifndef _parsers    // Put these two lines at the top of your file.
#define _parsers    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

String removeStartWord(const String &input , String wordToRemove);
void parseSpriteData(const String &input, uint8_t *&output, uint8_t &width, uint8_t &frames);
bool parseTime(String time, int &hour, int &minute, String &isAmPm);
bool parseDate(String date, int &year, int &month, int &day);
bool parseTime(String time, int &hour, int &minute , String &AmPm);
void parseTime24(String time, int &hour, int &minute);

#endif