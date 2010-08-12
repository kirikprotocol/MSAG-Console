#ifndef __SMSC_SYSTEM_VERSION__
#ident "@(#)$Id$"
#define __SMSC_SYSTEM_VERSION__

/* -- automatically generated BUILD number and date -- */
#include "system/smscBuildId.hpp"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define SMSC_VER_MAJOR     1    //major verison
#define SMSC_VER_MINOR     9    //minor version
#define SMSC_VER_FIX       0    //bug fix version
/* -- ----------------------------------------- -- */

#define SMSC_VENDOR        "Eyeline"
#define SMSC_PRODUCT_NAME  "SMSC"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern 
#endif

//version string in format that 'what' utility understands
EXTERN const char * _what_product_version;
//
EXTERN const char * getStrVersion(void);

#endif /* __SMSC_SYSTEM_VERSION__ */

