#include <Arduino.h>
#include <flashUtils/flashUtils.h>
#include <parsers/parsers.h>

const int ChatIDsLength = 10;
String ChatIDs[ChatIDsLength] = {"818675367"};

bool addUser(String users[] , int len , String newusesr)
{
  newusesr = removeStartWord(newusesr , "add");
  newusesr.trim();
  for (int i = 0; i < len; i++)
  {
    if (users[i] == "")
    {
      users[i] = newusesr;
      storeStringArray("usersArray", users , len);
      return true;
    }
  }
  return false;
}

bool removeUser(String users[] , int len , String needToRemoveUser)
{
  int Count = 0;
  for (int j = 0; j < len; j++)
  {
    if(users[j] != "")
    {
      Count++;
    }
  }
  if(Count == 0)
  {
    return false;
  }
  else
  {
    needToRemoveUser = removeStartWord(needToRemoveUser , "remove");
    needToRemoveUser.trim();
    for (int i = 0; i < len;  i++)
    {
      if (users[i] == needToRemoveUser)
      {
        users[i] = "";
        storeStringArray("usersArray", users , len);
        return true;
      }
    }
  }
  return false;
}

String listUsers(String users[] , int len)
{
  loadStringArray("usersArray" , users , len);
  String usersList = "";
  for (int i = 0; i < len; i++)
  {
    if(users[i] != "")
    {
      usersList += users[i] + "\n";
    }
  }
  return usersList;
}