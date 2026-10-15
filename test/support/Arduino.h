#pragma once
// A stand-in for the parts of Arduino's String that the pure-logic modules use, so they can be
// unit tested on a PC (pio test -e native). Behaviour follows the Arduino implementation.

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

class String
{
public:
  String() {}
  String(const char *s) : s_(s ? s : "") {}
  String(const std::string &s) : s_(s) {}
  String(char c) : s_(1, c) {}
  String(int v) : s_(std::to_string(v)) {}
  String(unsigned int v) : s_(std::to_string(v)) {}
  String(long v) : s_(std::to_string(v)) {}

  unsigned int length() const { return (unsigned int)s_.size(); }
  const char *c_str() const { return s_.c_str(); }
  char operator[](unsigned int i) const { return i < s_.size() ? s_[i] : 0; }

  bool startsWith(const String &p) const { return s_.compare(0, p.s_.size(), p.s_) == 0 && s_.size() >= p.s_.size(); }
  bool endsWith(const String &p) const { return s_.size() >= p.s_.size() && s_.compare(s_.size() - p.s_.size(), p.s_.size(), p.s_) == 0; }

  int indexOf(char c) const { return indexOf(c, 0); }
  int indexOf(char c, unsigned int from) const
  {
    if (from >= s_.size())
      return -1;
    size_t at = s_.find(c, from);
    return at == std::string::npos ? -1 : (int)at;
  }

  String substring(unsigned int left) const { return substring(left, length()); }
  String substring(unsigned int left, unsigned int right) const
  {
    if (left > right)
      std::swap(left, right);
    if (left >= s_.size())
      return String();
    if (right > s_.size())
      right = (unsigned int)s_.size();
    return String(s_.substr(left, right - left));
  }

  void remove(unsigned int index) { remove(index, (unsigned int)s_.size()); }
  void remove(unsigned int index, unsigned int count)
  {
    if (index < s_.size())
      s_.erase(index, count);
  }

  void trim()
  {
    size_t b = s_.find_first_not_of(" \t\r\n");
    size_t e = s_.find_last_not_of(" \t\r\n");
    s_ = (b == std::string::npos) ? "" : s_.substr(b, e - b + 1);
  }

  long toInt() const { return atol(s_.c_str()); }
  void toUpperCase() { std::transform(s_.begin(), s_.end(), s_.begin(), [](unsigned char c) { return std::toupper(c); }); }
  void toLowerCase() { std::transform(s_.begin(), s_.end(), s_.begin(), [](unsigned char c) { return std::tolower(c); }); }

  bool equalsIgnoreCase(const String &other) const
  {
    if (s_.size() != other.s_.size())
      return false;
    for (size_t i = 0; i < s_.size(); i++)
      if (std::tolower((unsigned char)s_[i]) != std::tolower((unsigned char)other.s_[i]))
        return false;
    return true;
  }

  String &operator+=(const String &o) { s_ += o.s_; return *this; }
  String &operator+=(char c) { s_ += c; return *this; }
  friend String operator+(const String &a, const String &b) { return String(a.s_ + b.s_); }
  bool operator==(const String &o) const { return s_ == o.s_; }
  bool operator!=(const String &o) const { return s_ != o.s_; }

private:
  std::string s_;
};
