#ifndef _parsers    // Put these two lines at the top of your file.
#define _parsers    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

String removeStartWord(const String &input , String wordToRemove);
// Parses "anim,<frames>,<width>,<hex bytes>" into a new array held in `output`.
// A frame is <width> bytes, one per column, least significant bit = top row (MD_Parola's format).
// `output` must be nullptr or the result of an earlier call: that array is freed first.
// Returns false (output = nullptr) if the text is malformed, too short or too long.
bool parseSpriteData(const String &input, uint8_t *&output, uint8_t &frames, uint8_t &width);
bool parseTime(String time, int &hour, int &minute, String &isAmPm);
bool parseDate(String date, int &year, int &month, int &day);
void parseTime24(String time, int &hour, int &minute);

#endif