#ifndef _usersManagement     // Put these two lines at the top of your file.
#define _usersManagement     // (Use a suitable name, usually based on the file name.)

#include <Arduino.h>   // Include Arduino core header to use the String class

bool addUser(String users[] , int len ,  String newusesr);
bool removeUser(String users[] , int len , String needToRemoveUser);
String listUsers(String users[] , int len);

extern const int ChatIDsLength;
extern String ChatIDs[10];

#endif