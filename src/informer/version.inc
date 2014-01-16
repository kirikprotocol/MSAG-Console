#include "version.def"
#define VER_MAKE_STR2(X) #X
#define VER_MAKE_STR(X) VER_MAKE_STR2(X)

#ifdef SNMP
#define SNMP_STR " snmp"
#else
#define SNMP_STR ""
#endif

#define VER_STR "Eyeline InfoSme ver " VER_MAKE_STR(VER_MAJOR) "." VER_MAKE_STR(VER_MINOR) " build " VER_BUILD " date " VER_DATE SNMP_STR

extern "C" {

const char* cssc_version="@(#)" VER_STR;

const char* getStrVersion()
{
  return VER_STR;
}

};
