/* ************************************************************************* *
 * USS Gateway (USSMan) version definition.
 * ************************************************************************* */
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

/* -- automatically generated BUILD number and date -- */
#include "ussman/ussgBuildId.hpp"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define USSMAN_VER_MAJOR     1    //major verison
#define USSMAN_VER_MINOR     1    //minor version
#define USSMAN_VER_FIX       4    //bug fix version
#define USSMAN_VER_TAG       ""   //release tag: [alpha, beta, RC]N
/* -- ----------------------------------------- -- */
#define USSMAN_VENDOR        "Eyeline"
#define USSMAN_PRODUCT_NAME  "USS Manager"
/* -- ----------------------------------------- -- */

#include "ussman/ussgversion.hpp"

#define DEF_TO_STR0(x) #x
#define DEF_TO_STR(x) DEF_TO_STR0(x)

#define USSMAN_VER_STR USSMAN_VENDOR " " USSMAN_PRODUCT_NAME " " DEF_TO_STR(USSMAN_VER_MAJOR) "." DEF_TO_STR(USSMAN_VER_MINOR) "." DEF_TO_STR(USSMAN_VER_FIX) USSMAN_VER_TAG " build " DEF_TO_STR(USSMAN_BUILD_NUM) " date " USSMAN_BUILD_DATE


const char * _what_product_version = "@(#)" USSMAN_VER_STR;

const char * _getProductVersionStr(void)
{
  return USSMAN_VER_STR;
}

