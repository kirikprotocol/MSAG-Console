#ifndef __EYELINE_SS7NA_COMMON_MESSAGE_HPP__
# define __EYELINE_SS7NA_COMMON_MESSAGE_HPP__

# include <string>
# include <sys/types.h>

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/MessageHandlingDispatcherIface.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class Message {
public:
  virtual ~Message() {}

  virtual size_t serialize(TP* result_buf) const = 0;

  virtual size_t deserialize(const TP& packet_buf) = 0;

  virtual std::string toString() const = 0;

  virtual uint32_t getMsgCode() const = 0;

  virtual const char* getMsgCodeTextDescription() const = 0;

  static const size_t TOTAL_NUM_OF_PARAMETERS=64;
  static const size_t MAX_MESSAGE_SIZE=TOTAL_NUM_OF_PARAMETERS*64*1024;

  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() { return NULL; }

protected:
  virtual uint32_t getLength() const = 0;

protected:
  // disable object creation in heap
  //static void operator delete (void*) {}
};

}}}

#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_MESSAGE>() {
  static TSD_Init_Helper<MEM_FOR_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(ss7na::common::Message::MAX_MESSAGE_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
