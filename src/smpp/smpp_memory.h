/*
  $Id$
*/

//
// этот файл содержит код управления выделением/освобождением памяти
//
#if !defined __Cxx_Header__smpp_memory_h__
#define __Cxx_Header__smpp_memory_h__

//#include <memory>
#include "util/debug.h"

namespace smsc{
namespace smpp{
/*
static inline void* smartMalloc(size_t size) { return malloc(size); }
static inline void smartFree(void* p) { free(p); }

static inline void* xx_new(size_t size)
{
  void* p = smartMalloc(size);
  __trace2__ ("new(%d) == %x", size,p);
  __require__ ( p != 0 );
  return p;
}

static inline void _delete(void* p)
{
  __require__ ( p != 0 );
  __trace2__ ("delete(%x)", p);
  smartFree(p);
}

extern void xx_delete(void* p);

static inline void* operator new(size_t size) {xx_new(size);}
static inline void* operator new[](size_t size) {xx_new(size);}
static inline void operator delete(void* p) {xx_delete(p);}
static inline void operator delete[](void* p) {xx_delete(p);}
*/
/*class MemoryManagerUnit
{
public:
  static inline void* operator new(size_t size) {xx_new(size);}
  static inline void* operator new[](size_t size) {xx_new(size);}
  static inline void operator delete(void* p) {xx_delete(p);}
  static inline void operator delete[](void* p) {xx_delete(p);}
};*/

};
};

#undef smartMalloc
#undef smartFree

#endif

