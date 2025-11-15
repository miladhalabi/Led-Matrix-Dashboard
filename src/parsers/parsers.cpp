#include <Arduino.h>

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

void parseSpriteData(const String &input, uint8_t *&output, uint8_t &width, uint8_t &frames)
{
  String cleanedInput = removeStartWord(input , "anim");
  const char *ptr = cleanedInput.c_str();
  width = strtol(ptr, (char **)&ptr, 10);
  ptr++;
  frames = strtol(ptr, (char **)&ptr, 10);
  ptr++;

  size_t outputSize = width * frames;
  output = new uint8_t[outputSize];

  for (size_t i = 0; i < outputSize; i++)
  {
    while (*ptr == ' ' || *ptr == ',')
    {
      ptr++;
    }
    uint8_t value = 0;
    for (int j = 0; j < 2; j++)
    {
      char c = *ptr++;
      value <<= 4;
      if (c >= '0' && c <= '9')
      {
        value |= (c - '0');
      }
      else if (c >= 'a' && c <= 'f')
      {
        value |= (c - 'a' + 10);
      }
      else if (c >= 'A' && c <= 'F')
      {
        value |= (c - 'A' + 10);
      }
    }
    output[i] = value;
  }
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

