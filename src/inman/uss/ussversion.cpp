#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/uss/ussversion.hpp"

#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define USSMAN_VER_STR USSMAN_VENDOR " " USSMAN_PRODUCT_NAME " " DEF_TO_STR(USSMAN_VER_MAJOR) "." DEF_TO_STR(USSMAN_VER_MINOR) "." DEF_TO_STR(USSMAN_VER_FIX) " build " DEF_TO_STR(USSMAN_BUILD_NUM) " date " USSMAN_BUILD_DATE


const char * _what_product_version = "@(#)" USSMAN_VER_STR;

const char * _getProductVersionStr(void)
{
  return USSMAN_VER_STR;
}

