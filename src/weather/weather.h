#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

extern String weatherTemp;
extern String weatherGlyph;

bool fetchWeather();

#endif
