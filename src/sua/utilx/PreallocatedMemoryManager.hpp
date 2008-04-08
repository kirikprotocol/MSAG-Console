#ifndef __SUA_UTILX_PREALLOCATEDMEMORYMANAGER_HPP__
# define __SUA_UTILX_PREALLOCATEDMEMORYMANAGER_HPP__ 1

# include <pthread.h>
# include <util/Singleton.hpp>
# include <util/Exception.hpp>
# include <sys/types.h>
# include <pthread.h>
# include <errno.h>

#include <logger/Logger.h>
extern smsc::logger::Logger *logger;

namespace utilx {

typedef struct {
  void* allocated_memory;
  size_t allocated_memory_size;
} alloc_mem_desc_t;

class PreallocatedMemoryManager : smsc::util::Singleton<PreallocatedMemoryManager> {
public:
  typedef enum { MEM_FOR_EVENT=1, MEM_FOR_MESSAGE=2, MEM_FOR_PACKET=3, MEM_FOR_ENCAPSULATED_MESSAGE=4 } mem_type_t;
  template<mem_type_t MEMTYPE> alloc_mem_desc_t* getMemory();

private:
  alloc_mem_desc_t* allocateMemory(size_t allocatingMemSize, pthread_key_t tsd);

  template<mem_type_t MEMTYP>
  struct TSD_Init_Helper {
    pthread_key_t _tsd_memory_key;
    static pthread_mutex_t _tsd_init_lock;
    static bool _wasInitialized;

    TSD_Init_Helper() {
      if ( !_wasInitialized ) {
        pthread_mutex_lock(&_tsd_init_lock);
        if ( !_wasInitialized ) {
          _wasInitialized = true;
          int errNum;
          if ( (errNum=pthread_key_create(&_tsd_memory_key, NULL)) != 0 ) {
            pthread_mutex_unlock(&_tsd_init_lock);
            throw SystemError("TSD_Init_Helper::TSD_Init_Helper::: call to pthread_key_create failed", errNum);
          }
        }
        pthread_mutex_unlock(&_tsd_init_lock);
      }
    }
  };
};


template <PreallocatedMemoryManager::mem_type_t MEMTYP>
pthread_mutex_t PreallocatedMemoryManager::TSD_Init_Helper<MEMTYP>::_tsd_init_lock=PTHREAD_MUTEX_INITIALIZER;

template <PreallocatedMemoryManager::mem_type_t MEMTYP>
bool PreallocatedMemoryManager::TSD_Init_Helper<MEMTYP>::_wasInitialized=false;

//#include <sua/utilx/PreallocatedMemoryManager_impl.hpp>

}

#endif
