#include "PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace utilx {

alloc_mem_desc_t*
PreallocatedMemoryManager::allocateMemory(size_t allocatingMemSize, pthread_key_t tsd) {
  errno = 0;
  alloc_mem_desc_t* mem =  (alloc_mem_desc_t*)pthread_getspecific(tsd);

  if ( !mem ) {
    uint8_t* _preallocated = new uint8_t[sizeof(alloc_mem_desc_t) + allocatingMemSize];
    mem = (alloc_mem_desc_t*)_preallocated;
    mem->allocated_memory = _preallocated + sizeof(alloc_mem_desc_t);
    mem->allocated_memory_size = allocatingMemSize;
    int errNum;
    if ( (errNum=pthread_setspecific(tsd, _preallocated)) ) {
      errno = errNum; return NULL;
    }
  }
  return mem;
}

}}
