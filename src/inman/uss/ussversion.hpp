#ifndef __SMSC_USSMAN_VERSION__
#ident "@(#)$Id$"
#define __SMSC_USSMAN_VERSION__

/* -- NOTE: these defines must be set by release builder -- */
#define USSMAN_BUILD_NUM   3118
#define USSMAN_BUILD_DATE  "Feb 01 2010"
/* -- -------------------------------------------------- -- */

/* -- NOTE: these defines are set by developers -- */
#define USSMAN_VER_MAJOR     0    //major verison
#define USSMAN_VER_MINOR     7    //minor version
#define USSMAN_VER_FIX       3    //bug fix version
/* -- ----------------------------------------- -- */

#define USSMAN_VENDOR        "Sibinco"
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

