#include "PreallocatedMemoryManager.hpp"
#include <logger/Logger.h>
namespace utilx {

alloc_mem_desc_t*
PreallocatedMemoryManager::allocateMemory(size_t allocatingMemSize, pthread_key_t tsd) {
  errno = 0;
  alloc_mem_desc_t* mem =  (alloc_mem_desc_t*)pthread_getspecific(tsd);
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("mem");

  //smsc_log_debug(logger, "PreallocatedMemoryManager::allocateMemory::: pthread_getspecific(tsd_key=%d) returned 0x%p, errno=%d, sizeof(alloc_mem_desc_t)=%d, allocatingMemSize=%d, sizeof(alloc_mem_desc_t) + allocatingMemSize=%d",tsd, mem, errno, sizeof(alloc_mem_desc_t), allocatingMemSize, sizeof(alloc_mem_desc_t) + allocatingMemSize);
  if ( !mem ) {
    uchar_t* _preallocated = new uchar_t[sizeof(alloc_mem_desc_t) + allocatingMemSize];
    mem = (alloc_mem_desc_t*)_preallocated;
    mem->allocated_memory = _preallocated + sizeof(alloc_mem_desc_t);
    mem->allocated_memory_size = allocatingMemSize;
    int errNum;
    if ( (errNum=pthread_setspecific(tsd, _preallocated)) ) {
      //      smsc_log_debug(logger, "PreallocatedMemoryManager::allocateMemory::: pthread_setspecific(tsd_key=%d,newMem=0x%p) returned %d",tsd, _preallocated, errNum);
      errno = errNum; return NULL;
    }
  }
  return mem;
}

}
