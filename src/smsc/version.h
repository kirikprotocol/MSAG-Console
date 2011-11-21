#ifndef __SMSC_VERSION_H__
#define __SMSC_VERSION_H__

/* -- automatically generated BUILD number and date -- */
#include "smsc/scalableSmscBuildId.hpp"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define SCALABLE_SMSC_VER_MAJOR     2    //major verison
#define SCALABLE_SMSC_VER_MINOR     0    //minor version
#define SCALABLE_SMSC_VER_FIX       0    //bug fix version
/* -- ----------------------------------------- -- */

#define SCALABLE_SMSC_VENDOR        "Eyeline"
#define SCALABLE_SMSC_PRODUCT_NAME  "Scalable SMSC"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern 
#endif

//version string in format that 'what' utility understands
EXTERN const char * _what_product_version;
//
EXTERN const char * getStrVersion(void);

#endif /* __SCALABLE_SMSC_2ND_GENERATION_VERSION__ */

