#include <stdlib.h>

#define VER_MAJOR "1"
#define VER_MINOR "0"
#define VER_BUILD "1"
#define VER_DATE  "Mon Nov  3 16:05:02 2003"

#define VER_STR "Sibinco SMSC ver " VER_MAJOR "." VER_MINOR " build " VER_BUILD " date " VER_DATE

extern "C" {

const char* cssc_version="@(#)" VER_STR;

const char* getStrVersion()
{
  return VER_STR;
}

void getVersion(int* major,int* minor,int* build)
{
  if(major)*major=atoi(VER_MAJOR);
  if(minor)*minor=atoi(VER_MINOR);
  if(build)*build=atoi(VER_BUILD);
}

};
