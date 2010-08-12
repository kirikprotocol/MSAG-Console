#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "smsc/version.h"

#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define SMSC_2G_VER_STR SMSC_2G_VENDOR " " SMSC_2G_PRODUCT_NAME " " DEF_TO_STR(SMSC_2G_VER_MAJOR) "." DEF_TO_STR(SMSC_2G_VER_MINOR) "." DEF_TO_STR(SMSC_2G_VER_FIX) " build " DEF_TO_STR(SMSC_2G_BUILD_NUM) " date " SMSC_2G_BUILD_DATE


const char * _what_product_version = "@(#)" SMSC_2G_VER_STR;

const char * getStrVersion(void)
{
  return SMSC_2G_VER_STR;
}

