#ifndef _utils    // Put these two lines at the top of your file.
#define _utils    // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class
#include <esp_task_wdt.h>

void monitorFreeHeapMemory(void *parameters);
bool longpress(int pin, int dtime, int state);
void resetEveryTwoHours();
int lenFull(String arr[] , int Len);


#endif