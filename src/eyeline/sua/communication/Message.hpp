#ifndef __EYELINE_SUA_COMMUNICATION_MESSAGE_HPP__
# define __EYELINE_SUA_COMMUNICATION_MESSAGE_HPP__

# include <string>
# include <sys/types.h>
# include <eyeline/sua/communication/TP.hpp>
# include <eyeline/sua/communication/MessageHandlingDispatcherIface.hpp>

namespace eyeline {
namespace sua {
namespace communication {

class LinkId;

class Message {
public:
  virtual ~Message() {}

  virtual size_t serialize(TP* resultBuf) const = 0;

  virtual size_t deserialize(const TP& packetBuf) = 0;

  virtual std::string toString() const = 0;

  virtual uint32_t getMsgCode() const = 0;

  virtual const char* getMsgCodeTextDescription() const = 0;

  static const size_t TOTAL_NUM_OF_PARAMETERS=64;
  static const size_t MAX_MESSAGE_SIZE=TOTAL_NUM_OF_PARAMETERS*64*1024;

  virtual MessageHandlingDispatcherIface* getHandlingDispatcher();

protected:
  virtual uint32_t getLength() const = 0;

  static unsigned int generateMessageIndexValue();

private:
  static unsigned int _MSG_IDX;

protected:
  // disable object creation in heap
  static void operator delete (void*) {}
};

}}} // namespace eyeline::sua::communication

#include <eyeline/utilx/PreallocatedMemoryManager.hpp>

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_MESSAGE>() {
  static TSD_Init_Helper<MEM_FOR_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(sua::communication::Message::MAX_MESSAGE_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
