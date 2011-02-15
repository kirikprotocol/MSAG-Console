#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

/* -- automatically generated BUILD number and date -- */
#include "system/smscBuildId.hpp"
/* -- -------------------------------------------------- -- */

#include "system/version.h"

/* -- NOTE: these defines are set by developers -- */
#define SMSC_VER_MAJOR     1    //major verison
#define SMSC_VER_MINOR     9    //minor version
#define SMSC_VER_FIX       0    //bug fix version
/* -- ----------------------------------------- -- */

#define SMSC_VENDOR        "Eyeline"
#define SMSC_PRODUCT_NAME  "SMSC"


#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define SMSC_VER_STR SMSC_VENDOR " " SMSC_PRODUCT_NAME " " DEF_TO_STR(SMSC_VER_MAJOR) "." DEF_TO_STR(SMSC_VER_MINOR) "." DEF_TO_STR(SMSC_VER_FIX) " build " DEF_TO_STR(SMSC_BUILD_NUM) " date " SMSC_BUILD_DATE


const char * _what_product_version = "@(#)" SMSC_VER_STR;

const char * getStrVersion(void)
{
  return SMSC_VER_STR;
}

