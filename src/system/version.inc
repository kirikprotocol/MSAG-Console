
#define VER_MAJOR 1
#define VER_MINOR 0
#define VER_BUILD 1604
#define STR2(X) #X
#define STR(X) STR2(X)
#define VER_DATE  "Tue Nov  4 10:51:36 2003"

#define VER_STR "Sibinco SMSC ver " STR(VER_MAJOR) "." STR(VER_MINOR) " build " STR(VER_BUILD) " date " VER_DATE

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
