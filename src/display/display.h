#ifndef _display    // Put these two lines at the top of your file.
#define _display    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <MD_Parola.h>

extern MD_Parola myDisplay;
extern uint8_t customChar[];
extern uint8_t customChar1[];
extern uint8_t sunGlyph[];
extern uint8_t cloudGlyph[];
extern uint8_t rainGlyph[];
extern uint8_t moonGlyph[];
void handleDisplayTask(void *parameter);


#endif