#include "textUtils.h"
#include <ctype.h>

String firstWord(const String &text)
{
  for (unsigned int i = 0; i < text.length(); i++)
  {
    if (text[i] == ' ' || text[i] == ',')
      return text.substring(0, i);
  }
  return text;
}

bool isSha256(const String &text)
{
  if (text.length() != 64)
    return false;
  for (unsigned int i = 0; i < text.length(); i++)
  {
    if (!isxdigit((unsigned char)text[i]))
      return false;
  }
  return true;
}

String toHex(const uint8_t *bytes, size_t length)
{
  static const char digits[] = "0123456789abcdef";
  String hex;
  for (size_t i = 0; i < length; i++)
  {
    hex += digits[bytes[i] >> 4];
    hex += digits[bytes[i] & 0x0F];
  }
  return hex;
}
