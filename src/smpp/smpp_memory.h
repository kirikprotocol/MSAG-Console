
#define smartMalloc(x) malloc(x)
#define smartFree(x)   free(x)

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

#undef smartMalloc
#undef smartFree

