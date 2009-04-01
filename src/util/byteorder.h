#ifndef _UTIL_BYTEORDER_H
#define _UTIL_BYTEORDER_H

// the appearance of this file in our code is due to brain-dead solaris system headers.
// they even don't have a dedicated header for such useful thing as endianness !

// So, the following was stolen from SunOs:/usr/include/arpa/nameser_compat.h

#ifndef BYTE_ORDER
#if (BSD >= 199103)
#include <machine/endian.h>
#else
#ifdef linux
#include <endian.h>
#else

#define	LITTLE_ENDIAN	1234	/* least-significant byte first (vax, pc) */
#define	BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#if defined(vax) || defined(ns32000) || defined(sun386) || defined(i386) || \
	defined(__i386) || defined(__ia64) || defined(__amd64) || \
	defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
	defined(__alpha__) || defined(__alpha) || \
	(defined(__Lynx__) && defined(__x86__))
#define	BYTE_ORDER	LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(__sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined(__hp3000s900) || defined(MPE) || \
    defined(BIT_ZERO_ON_LEFT) || defined(m68k) || \
	(defined(__Lynx__) && \
	(defined(__68k__) || defined(__sparc__) || defined(__powerpc__)))
#define	BYTE_ORDER	BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

#ifndef BYTE_ORDER
#ifdef __BYTE_ORDER
#define BYTE_ORDER  __BYTE_ORDER
#endif
#endif

#ifndef LITTLE_ENDIAN
#ifdef __LITTLE_ENDIAN
#define LITTLE_ENDIAN  __LITTLE_ENDIAN
#endif
#endif

#ifndef BIG_ENDIAN
#ifndef __BIG_ENDIAN
#define BIG_ENDIAN  __BIG_ENDIAN
#endif
#endif

#if !defined(BYTE_ORDER) || \
	(BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN && \
    BYTE_ORDER != PDP_ENDIAN)
	/*
	 * you must determine what the correct bit order is for
	 * your compiler - the next line is an intentional error
	 * which will force your compiles to bomb until you fix
	 * the above macros.
	 */
	error "Undefined or invalid BYTE_ORDER";
#endif

#endif /* _UTIL_BYTEORDER_H */
