#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_IOEVENT_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_IOEVENT_HPP__

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class IOEvent {
public:
  virtual ~IOEvent() {}
  virtual void handle() = 0;
protected:
  static void operator delete (void*) {}
};

}}}}

#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_EVENT>() {
  static const int MAX_EVENT_SIZE=1024;
  static TSD_Init_Helper<MEM_FOR_EVENT> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(MAX_EVENT_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
