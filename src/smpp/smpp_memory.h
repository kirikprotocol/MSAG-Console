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


inline void* smartMalloc(size_t size) { return malloc(size); }
inline void smartFree(void* p) { free(p); }

class MemoryManagerUnit
{
public:	
	static inline void* _new(size_t size)
	{ 
		void* p = smartMalloc(size);
		__trace2__ ("new(%d) == %x", size,p);
		__require__ ( p != 0 );
		return p;
	}
	static inline void* operator new(size_t size) {_new(size);}
	static inline void* operator new[](size_t size) {_new(size);}
	
	static inline void _delete(void* p)
	{ 
		__require__ ( p != 0 );
		__trace2__ ("delete(%x)", p);
		smartFree(p);
	}
	static inline void operator delete(void* p) {_delete(p);}
	static inline void operator delete[](void* p) {_delete(p);}
};

};
};

#undef smartMalloc
#undef smartFree

#endif

