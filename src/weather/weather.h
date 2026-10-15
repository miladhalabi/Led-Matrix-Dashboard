#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <shared/shared_string.h>

extern SharedString weatherTemp;
extern SharedString weatherGlyph;

bool fetchWeather();

#endif
