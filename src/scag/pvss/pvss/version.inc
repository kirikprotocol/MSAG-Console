#include "version.def"
#define VER_MAKE_STR2(X) #X
#define VER_MAKE_STR(X) VER_MAKE_STR2(X)

#define VER_STR "Eyeline PVSS ver " VER_MAKE_STR(VER_MAJOR) "." VER_MAKE_STR(VER_MINOR) " build " VER_BUILD " date " VER_DATE

extern "C" {

const char* cssc_version="@(#)" VER_STR;

const char* getStrVersion()
{
  return VER_STR;
}

/*
* function getVersion doesnt use anywhere!
*
*/
void getVersion(int* major,int* minor,int* build)
{
  if(major)*major=VER_MAJOR;
  if(minor)*minor=VER_MINOR;
  if(build)*build=0; //VER_BUILD; to avoid error, jenkins VER_BUILD is a string
}

};
