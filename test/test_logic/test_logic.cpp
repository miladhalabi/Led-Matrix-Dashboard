#include <Arduino.h>
#include <unity.h>

// Pure-logic modules under test (see src/).
int nat(int number);
String longToString(long number);
void convertTo24Hour(int hour12, int minute, const String &period, int &hour24, int &minute24);
String formatTime12Hour(int hour, int minute, bool &isPM);
String removeStartWord(const String &input, String wordToRemove);
bool parseTime(String time, int &hour, int &minute, String &isAmPm);
void parseTime24(String time, int &hour, int &minute);
bool parseSpriteData(const String &input, uint8_t *&output, uint8_t &width, uint8_t &frames);
String firstWord(const String &text);
bool isSha256(const String &text);
String toHex(const uint8_t *bytes, size_t length);
bool parseDate(String date, int &year, int &month, int &day);

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------- converters
void test_nat_flips_zero_and_one()
{
  TEST_ASSERT_EQUAL(1, nat(0));
  TEST_ASSERT_EQUAL(0, nat(1));
  TEST_ASSERT_EQUAL(0, nat(7));
}

void test_longToString()
{
  TEST_ASSERT_EQUAL_STRING("42", longToString(42).c_str());
  TEST_ASSERT_EQUAL_STRING("-5", longToString(-5).c_str());
}

static void to24(int h, int m, const char *period, int expectH, int expectM)
{
  int h24 = -1, m24 = -1;
  convertTo24Hour(h, m, period, h24, m24);
  TEST_ASSERT_EQUAL(expectH, h24);
  TEST_ASSERT_EQUAL(expectM, m24);
}

void test_convertTo24Hour()
{
  to24(12, 30, "AM", 0, 30);  // midnight hour
  to24(12, 0, "PM", 12, 0);   // noon
  to24(1, 15, "PM", 13, 15);
  to24(11, 59, "PM", 23, 59);
  to24(11, 59, "AM", 11, 59);
  to24(1, 0, "AM", 1, 0);
}

// ---------------------------------------------------------------- formaters
static void formatsAs(int hour, int minute, const char *expected, bool expectPm)
{
  bool pm = !expectPm;
  TEST_ASSERT_EQUAL_STRING(expected, formatTime12Hour(hour, minute, pm).c_str());
  TEST_ASSERT_EQUAL(expectPm, pm);
}

void test_formatTime12Hour()
{
  formatsAs(0, 5, "12:05", false);
  formatsAs(9, 0, "9:00", false);
  formatsAs(12, 0, "12:00", true);
  formatsAs(13, 7, "1:07", true);
  formatsAs(23, 59, "11:59", true);
}

// ---------------------------------------------------------------- parsers
void test_removeStartWord()
{
  TEST_ASSERT_EQUAL_STRING("07:00:am", removeStartWord("alarm,07:00:am", "alarm").c_str());
  TEST_ASSERT_EQUAL_STRING("on", removeStartWord("alarm on", "alarm").c_str());
  TEST_ASSERT_EQUAL_STRING("hello", removeStartWord("hello", "alarm").c_str());
}

void test_parseTime_accepts_12_hour_times()
{
  int h = 0, m = 0;
  String period;
  TEST_ASSERT_TRUE(parseTime("07:30:am", h, m, period));
  TEST_ASSERT_EQUAL(7, h);
  TEST_ASSERT_EQUAL(30, m);
  TEST_ASSERT_EQUAL_STRING("AM", period.c_str());
  TEST_ASSERT_TRUE(parseTime("12:00:PM", h, m, period));
  TEST_ASSERT_EQUAL_STRING("PM", period.c_str());
}

void test_parseTime_rejects_bad_input()
{
  int h = 0, m = 0;
  String period;
  TEST_ASSERT_FALSE(parseTime("13:00:pm", h, m, period)); // hour out of range
  TEST_ASSERT_FALSE(parseTime("0:30:am", h, m, period));
  TEST_ASSERT_FALSE(parseTime("7:60:am", h, m, period));  // minute out of range
  TEST_ASSERT_FALSE(parseTime("7:30", h, m, period));     // no AM/PM
  TEST_ASSERT_FALSE(parseTime("7:30:xx", h, m, period));
}

void test_parseTime24()
{
  int h = 0, m = 0;
  parseTime24("04:35", h, m);
  TEST_ASSERT_EQUAL(4, h);
  TEST_ASSERT_EQUAL(35, m);
  parseTime24("23:59", h, m);
  TEST_ASSERT_EQUAL(23, h);
  TEST_ASSERT_EQUAL(59, m);
}

void test_parseDate()
{
  int y = 0, mo = 0, d = 0;
  TEST_ASSERT_TRUE(parseDate("2025/08/22", y, mo, d));
  TEST_ASSERT_EQUAL(2025, y);
  TEST_ASSERT_EQUAL(8, mo);
  TEST_ASSERT_EQUAL(22, d);
  TEST_ASSERT_FALSE(parseDate("2025/02/30", y, mo, d));
  TEST_ASSERT_FALSE(parseDate("2025/13/01", y, mo, d));
  TEST_ASSERT_FALSE(parseDate("2025/2", y, mo, d));
  TEST_ASSERT_FALSE(parseDate("999/01/01", y, mo, d));
}

// ---------------------------------------------------------------- animations
void test_parseSpriteData_reads_width_frames_and_bytes()
{
  uint8_t *data = nullptr;
  uint8_t width = 0, frames = 0;
  TEST_ASSERT_TRUE(parseSpriteData("anim,2,2,01,0a 0B,ff", data, width, frames));
  TEST_ASSERT_EQUAL(2, width);
  TEST_ASSERT_EQUAL(2, frames);
  TEST_ASSERT_NOT_NULL(data);
  TEST_ASSERT_EQUAL_HEX8(0x01, data[0]);
  TEST_ASSERT_EQUAL_HEX8(0x0A, data[1]);
  TEST_ASSERT_EQUAL_HEX8(0x0B, data[2]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, data[3]);
  delete[] data;
}

void test_parseSpriteData_replaces_the_previous_animation()
{
  uint8_t *data = nullptr;
  uint8_t width = 0, frames = 0;
  TEST_ASSERT_TRUE(parseSpriteData("anim,1,1,aa", data, width, frames));
  TEST_ASSERT_TRUE(parseSpriteData("anim,1,2,bb,cc", data, width, frames)); // frees the first array
  TEST_ASSERT_EQUAL(2, frames);
  TEST_ASSERT_EQUAL_HEX8(0xCC, data[1]);
  delete[] data;
}

void test_parseSpriteData_rejects_bad_input()
{
  uint8_t *data = nullptr;
  uint8_t width = 0, frames = 0;
  TEST_ASSERT_FALSE(parseSpriteData("anim,2,2,01,02", data, width, frames));    // too little data
  TEST_ASSERT_FALSE(parseSpriteData("anim,1,1,zz", data, width, frames));       // not hex
  TEST_ASSERT_FALSE(parseSpriteData("anim,1,1,a", data, width, frames));        // half a byte
  TEST_ASSERT_FALSE(parseSpriteData("anim,0,4,00", data, width, frames));       // empty
  TEST_ASSERT_FALSE(parseSpriteData("anim,255,255,00", data, width, frames));   // too large
  TEST_ASSERT_FALSE(parseSpriteData("anim,x,2,00", data, width, frames));
  TEST_ASSERT_FALSE(parseSpriteData("anim", data, width, frames));
  TEST_ASSERT_NULL(data);
}

// ---------------------------------------------------------------- text helpers
void test_firstWord()
{
  TEST_ASSERT_EQUAL_STRING("alarm", firstWord("alarm,07:00:am").c_str());
  TEST_ASSERT_EQUAL_STRING("alarm", firstWord("alarm on").c_str());
  TEST_ASSERT_EQUAL_STRING("ring", firstWord("ring").c_str());
  TEST_ASSERT_EQUAL_STRING("", firstWord("").c_str());
  TEST_ASSERT_EQUAL_STRING("", firstWord(" leading space").c_str());
}

void test_isSha256()
{
  TEST_ASSERT_TRUE(isSha256("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
  TEST_ASSERT_TRUE(isSha256("E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"));
  TEST_ASSERT_FALSE(isSha256("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b85"));   // 63
  TEST_ASSERT_FALSE(isSha256("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b8555")); // 65
  TEST_ASSERT_FALSE(isSha256("g3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));  // not hex
  TEST_ASSERT_FALSE(isSha256(""));
}

void test_toHex()
{
  const uint8_t bytes[] = {0x00, 0xAB, 0xFF, 0x10};
  TEST_ASSERT_EQUAL_STRING("00abff10", toHex(bytes, sizeof(bytes)).c_str());
  TEST_ASSERT_EQUAL_STRING("", toHex(bytes, 0).c_str());
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_nat_flips_zero_and_one);
  RUN_TEST(test_longToString);
  RUN_TEST(test_convertTo24Hour);
  RUN_TEST(test_formatTime12Hour);
  RUN_TEST(test_removeStartWord);
  RUN_TEST(test_parseTime_accepts_12_hour_times);
  RUN_TEST(test_parseTime_rejects_bad_input);
  RUN_TEST(test_parseTime24);
  RUN_TEST(test_parseDate);
  RUN_TEST(test_parseSpriteData_reads_width_frames_and_bytes);
  RUN_TEST(test_parseSpriteData_replaces_the_previous_animation);
  RUN_TEST(test_parseSpriteData_rejects_bad_input);
  RUN_TEST(test_firstWord);
  RUN_TEST(test_isSha256);
  RUN_TEST(test_toHex);
  return UNITY_END();
}
