#ifndef _encoder    // Put these two lines at the top of your file.
#define _encoder    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <ESP32Encoder.h>


extern ESP32Encoder encoder;

extern int brightness;
extern int AlarmVolume;
extern int AlarmMusic;
extern String alarmState;
extern String prayState;
extern int hours , minutes, day, month, year, amPm;


void handleEncoder();
void handleEncoderTask(void *parameter);


#endif