#ifndef _textUtils
#define _textUtils

#include <Arduino.h>

/** The first word of `text`: everything before the first space or comma ("alarm,07:00:am" -> "alarm"). */
String firstWord(const String &text);

/** True for exactly 64 hex digits, i.e. a SHA-256 written as text. */
bool isSha256(const String &text);

/** Lower-case hex of `length` bytes, e.g. {0x00, 0xAB} -> "00ab". */
String toHex(const uint8_t *bytes, size_t length);

#endif
