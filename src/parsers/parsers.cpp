#include <Arduino.h>
#include <new>

String removeStartWord(const String &input , String wordToRemove)
{
  String result = input;
  if (result.startsWith(wordToRemove))
  {
    result.remove(0, wordToRemove.length());
    result.trim();
    if (result.startsWith(","))
    {
      result.remove(0, 1);
    }
  }
  return result;
}

// Longest animation we accept (width x frames bytes). The built-in ones are well under 200.
static const long MAX_SPRITE_BYTES = 512;

static int hexValue(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

bool parseSpriteData(const String &input, uint8_t *&output, uint8_t &width, uint8_t &frames)
{
  delete[] output; // the previous animation; without this every call leaked its buffer
  output = nullptr;

  const String text = removeStartWord(input, "anim");
  const char *ptr = text.c_str();
  char *end = nullptr;

  const long newWidth = strtol(ptr, &end, 10);
  if (end == ptr || (*end != ',' && *end != ' '))
    return false;
  ptr = end + 1;

  const long newFrames = strtol(ptr, &end, 10);
  if (end == ptr || (*end != ',' && *end != ' '))
    return false;
  ptr = end + 1;

  if (newWidth < 1 || newFrames < 1 || newWidth * newFrames > MAX_SPRITE_BYTES)
    return false;

  const size_t size = (size_t)(newWidth * newFrames);
  uint8_t *data = new (std::nothrow) uint8_t[size];
  if (!data)
    return false;

  for (size_t i = 0; i < size; i++)
  {
    while (*ptr == ' ' || *ptr == ',')
      ptr++;
    const int high = hexValue(ptr[0]);
    const int low = high < 0 ? -1 : hexValue(ptr[1]); // never reads past the end of the text
    if (high < 0 || low < 0)
    {
      delete[] data; // malformed or too short
      return false;
    }
    data[i] = (uint8_t)((high << 4) | low);
    ptr += 2;
  }

  output = data;
  width = (uint8_t)newWidth;
  frames = (uint8_t)newFrames;
  return true;
}

bool parseTime(String time, int &hour, int &minute, String &isAmPm)
 {
  int firstColonIndex = time.indexOf(':');
  int secondColonIndex = time.indexOf(':', firstColonIndex + 1);

  if (firstColonIndex == -1 || secondColonIndex == -1) {
    return false;
  }

  hour = time.substring(0, firstColonIndex).toInt();
  minute = time.substring(firstColonIndex + 1, secondColonIndex).toInt();
  isAmPm = time.substring(secondColonIndex + 1);
  isAmPm.trim();
  isAmPm.toUpperCase();
  if (hour < 1 || hour > 12 || minute < 0 || minute > 59) {
    return false;
  }
  if (!(isAmPm.equalsIgnoreCase("AM") || isAmPm.equalsIgnoreCase("PM"))) {
    return false;
  }
  return true;
}

void parseTime24(String time, int &hour, int &minute)
 {
  int firstColonIndex = time.indexOf(':');
  int secondColonIndex = time.indexOf(':', firstColonIndex + 1);
  hour = time.substring(0, firstColonIndex).toInt();
  minute = time.substring(firstColonIndex + 1, secondColonIndex).toInt();
}

bool parseDate(String date, int &year, int &month, int &day)
{

  int firstSlash = date.indexOf('/');
  int secondSlash = date.indexOf('/', firstSlash + 1);
  
  if (firstSlash == -1 || secondSlash == -1 || secondSlash <= firstSlash) {
    return false; 
  }

  year = date.substring(0, firstSlash).toInt();
  month = date.substring(firstSlash + 1, secondSlash).toInt();
  day = date.substring(secondSlash + 1).toInt();

  if (year < 1000 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) {
    return false;
  }
  int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (day > daysInMonth[month - 1]) {
    return false;
  }

  return true;
}

