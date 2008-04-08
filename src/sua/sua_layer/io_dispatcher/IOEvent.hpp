#ifndef IO_DISPATCHER_IOEVENT_HPP_HEADER_INCLUDED_B87B69E0
# define IO_DISPATCHER_IOEVENT_HPP_HEADER_INCLUDED_B87B69E0

namespace io_dispatcher {

class IOEvent {
public:
  virtual ~IOEvent();
  virtual void handle() = 0;
protected:
  static void operator delete (void*) {}
};

} // namespace io_dispatcher


#include <sua/utilx/PreallocatedMemoryManager.hpp>

namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_EVENT>() {
  static const int MAX_MESSAGE_SIZE=1024;
  static TSD_Init_Helper<MEM_FOR_EVENT> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(MAX_MESSAGE_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}

#endif /* IO_DISPATCHER_IOEVENT_HPP_HEADER_INCLUDED_B87B69E0 */
