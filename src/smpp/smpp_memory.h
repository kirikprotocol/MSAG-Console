/*
	$Id$
*/

//
// этот файл содержит код управления выделением/освобождением памяти
//
#if !defined __Cxx_Header__smpp_memory_h__
#define __Cxx_Header__smpp_memory_h__

#include <memory>
#include "util/debug.h"

namespace smsc{
namespace smpp{


inline void* smartMalloc(size_t size) { return malloc(size); }
inline void smartFree(void* p) { free(p); }

class MemoryManagerUnit
{
	inline void* operator new(size_t size) 
	{ 
		void* p = smartMalloc(size);
		trace ("new(%d) == %x", size,p);
		__require__ ( p != 0 );
		return p;
	}
	inline void* operator new[](size_t size) {operator new(size)};
	inline void operator delete(void* p) 
	{ 
		__require__ ( p != 0 )
		trace ("delete(%x)", p);
		smartFree(p);
	}
	inline void operator delete[](void*) {operator delete(p)};
};

};
};

#undef smartMalloc
#undef smartFree

#endif

