#ifndef __SMSC_USSMAN_VERSION__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_VERSION__

/* -- automatically generated BUILD number and date -- */
#include "inman/uss/ussBuildId.hpp"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define USSMAN_VER_MAJOR     0    //major verison
#define USSMAN_VER_MINOR     7    //minor version
#define USSMAN_VER_FIX       4    //bug fix version
/* -- ----------------------------------------- -- */

#define USSMAN_VENDOR        "Eyeline"
#define USSMAN_PRODUCT_NAME  "USS Manager"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern 
#endif

//version string in format that 'what' utility understands
EXTERN const char * _what_product_version;
//
EXTERN const char * _getProductVersionStr(void);


#endif /* __SMSC_USSMAN_VERSION__ */

