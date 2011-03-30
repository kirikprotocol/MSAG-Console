
#define VER_MAJOR 3
#define VER_MINOR 1
#define VER_BUILD 315
#define VER_MAKE_STR2(X) #X
#define VER_MAKE_STR(X) VER_MAKE_STR2(X)
#define VER_DATE  "Mar 14 2011"

#define VER_STR "Sibinco MissedCallsInfo SMPP service ver " VER_MAKE_STR(VER_MAJOR) "." VER_MAKE_STR(VER_MINOR) " build " VER_MAKE_STR(VER_BUILD) " date " VER_DATE

extern "C" {

const char* cssc_version="@(#)" VER_STR;

const char* getStrVersion()
{
  return VER_STR;
}

void getVersion(int* major,int* minor,int* build)
{
  if(major)*major=VER_MAJOR;
  if(minor)*minor=VER_MINOR;
  if(build)*build=VER_BUILD;
}

};
