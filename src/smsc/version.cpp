#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "smsc/version.h"

#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define SCALABLE_SMSC_VER_STR SCALABLE_SMSC_VENDOR " " SCALABLE_SMSC_PRODUCT_NAME " " \
  DEF_TO_STR(SCALABLE_SMSC_VER_MAJOR) "." DEF_TO_STR(SCALABLE_SMSC_VER_MINOR) "." \
  DEF_TO_STR(SCALABLE_SMSC_VER_FIX) " build " DEF_TO_STR(SCALABLE_SMSC_BUILD_NUM) " date " SCALABLE_SMSC_BUILD_DATE


const char * _what_product_version = "@(#)" SCALABLE_SMSC_VER_STR;

const char * getStrVersion(void)
{
  return SCALABLE_SMSC_VER_STR;
}

