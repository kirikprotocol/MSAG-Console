#ifndef _ASN_INTERNAL_H_
#define _ASN_INTERNAL_H_

#define ASN1C_ENVIRONMENT_VERSION 98  /* Compile-time version */
int get_asn1c_environment_version(void);  /* Run-time version */

#include <asn_application.h>  /* Application-visible API */

#define CALLOC(nmemb, size) calloc(nmemb, size)
#define MALLOC(size)    malloc(size)
#define REALLOC(oldptr, size) realloc(oldptr, size)
#define FREEMEM(ptr)    free(ptr)

/*
 * A macro for debugging the ASN.1 internals.
 * You may enable or override it.
 */

//#define EMIT_ASN_DEBUG 1
#ifndef ASN_DEBUG /* If debugging code is not defined elsewhere... */
#if EMIT_ASN_DEBUG == 1 /* And it was asked to emit this code... */
#ifdef  __GNUC__
#define ASN_DEBUG(fmt, args...) do {    \
    fprintf(stderr, fmt, ##args); \
    fprintf(stderr, " (%s:%d)\n", \
      __FILE__, __LINE__);  \
  } while(0)
#else /* !__GNUC__ */
void ASN_DEBUG_f(const char *fmt, ...);
#define ASN_DEBUG ASN_DEBUG_f
#endif  /* __GNUC__ */
#else /* EMIT_ASN_DEBUG != 1 */
#ifdef  __GNUC__
#define ASN_DEBUG(fmt, args...) ((void)0) /* Emit a no-op operator */
#else /* __GNUC__ */
/*static void ASN_DEBUG(const char *fmt, ...) { (void)fmt; }; */
#define ASN_DEBUG(fmt, ...) 
#endif  /* __GNUC__ */
#endif  /* EMIT_ASN_DEBUG */
#endif  /* ASN_DEBUG */

/*
 * Invoke the application-supplied callback and fail, if something is wrong.
 */
#define __ASN_E_cbc(buf, size)  (cb((buf), (size), app_key) < 0)
#define _ASN_E_CALLBACK(foo)  do {          \
    if(foo) goto cb_failed;         \
  } while(0)
#define _ASN_CALLBACK(buf, size)          \
  _ASN_E_CALLBACK(__ASN_E_cbc(buf, size))
#define _ASN_CALLBACK2(buf1, size1, buf2, size2)      \
  _ASN_E_CALLBACK(__ASN_E_cbc(buf1, size1) || __ASN_E_cbc(buf2, size2))
#define _ASN_CALLBACK3(buf1, size1, buf2, size2, buf3, size3)   \
  _ASN_E_CALLBACK(__ASN_E_cbc(buf1, size1)      \
    || __ASN_E_cbc(buf2, size2)       \
    || __ASN_E_cbc(buf3, size3))

#define _i_ASN_TEXT_INDENT(nl, level) do {        \
  int __level = (level);            \
  int __nl = ((nl) != 0);           \
  int __i;              \
  if(__nl) _ASN_CALLBACK("\n", 1);        \
  for(__i = 0; __i < __level; __i++)        \
    _ASN_CALLBACK("    ", 4);       \
  er.encoded += __nl + 4 * __level;       \
} while(0)

#define _i_INDENT(nl) do {            \
  int __i;              \
  if((nl) && cb("\n", 1, app_key) < 0) return -1;     \
  for(__i = 0; __i < ilevel; __i++)       \
    if(cb("    ", 4, app_key) < 0) return -1;   \
} while(0)

#endif  /* _ASN_INTERNAL_H_ */
