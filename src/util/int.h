#ifndef __INT_H__
#define __INT_H__
#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#ifdef __GNUC__
typedef unsigned long ulong_t;
#endif
#endif
#endif
