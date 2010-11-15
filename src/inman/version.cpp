#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/version.hpp"

#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define INMAN_VER_STR INMAN_VENDOR " " INMAN_PRODUCT_NAME " " DEF_TO_STR(INMAN_VER_MAJOR) "." DEF_TO_STR(INMAN_VER_MINOR) "." DEF_TO_STR(INMAN_VER_FIX) " build " DEF_TO_STR(INMAN_BUILD_NUM) " date " INMAN_BUILD_DATE


const char * _what_product_version = "@(#)" INMAN_VER_STR;

const char * _getProductVersionStr(void)
{
  return INMAN_VER_STR;
}

