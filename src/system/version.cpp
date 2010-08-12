#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "system/version.h"

#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define SMSC_VER_STR SMSC_VENDOR " " SMSC_PRODUCT_NAME " " DEF_TO_STR(SMSC_VER_MAJOR) "." DEF_TO_STR(SMSC_VER_MINOR) "." DEF_TO_STR(SMSC_VER_FIX) " build " DEF_TO_STR(SMSC_BUILD_NUM) " date " SMSC_BUILD_DATE


const char * _what_product_version = "@(#)" SMSC_VER_STR;

const char * getStrVersion(void)
{
  return SMSC_VER_STR;
}

