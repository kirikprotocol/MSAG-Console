#ifndef __SMSC_INMAN_VERSION__
#ident "@(#)$Id$"
#define __SMSC_INMAN_VERSION__

/* -- NOTE: these defines must be set by release builder -- */
#define INMAN_BUILD_NUM   3117
#define INMAN_BUILD_DATE  "Dec 12 2009"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define INMAN_VER_MAJOR     0    //major verison
#define INMAN_VER_MINOR     7    //minor version
#define INMAN_VER_FIX       11   //bug fix version
/* -- ----------------------------------------- -- */

#define INMAN_VENDOR        "Sibinco"
#define INMAN_PRODUCT_NAME  "IN Manager"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern 
#endif

//version string in format that 'what' utility understands
EXTERN const char * _what_product_version;
//
EXTERN const char * _getProductVersionStr(void);

#endif /* __SMSC_INMAN_VERSION__ */

