#ifndef __SMSC_2ND_GENERATION_VERSION__
#ident "@(#)$Id$"
#define __SMSC_2ND_GENERATION_VERSION__

/* -- automatically generated BUILD number and date -- */
#include "smsc/smsc2gBuildId.hpp"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define SMSC_2G_VER_MAJOR     2    //major verison
#define SMSC_2G_VER_MINOR     0    //minor version
#define SMSC_2G_VER_FIX       0    //bug fix version
/* -- ----------------------------------------- -- */

#define SMSC_2G_VENDOR        "Eyeline"
#define SMSC_2G_PRODUCT_NAME  "SMSC_2G"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern 
#endif

//version string in format that 'what' utility understands
EXTERN const char * _what_product_version;
//
EXTERN const char * getStrVersion(void);

#endif /* __SMSC_2G_2ND_GENERATION_VERSION__ */

