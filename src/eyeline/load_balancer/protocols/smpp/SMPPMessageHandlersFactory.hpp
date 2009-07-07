#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_SMPPMESSAGEHANDLERSFACTORY_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_SMPPMESSAGEHANDLERSFACTORY_HPP__

# include <sys/types.h>

# include "logger/Logger.h"
# include "util/Exception.hpp"
# include "eyeline/load_balancer/io_subsystem/MessageHandlersFactory.hpp"
# include "eyeline/load_balancer/protocols/smpp/types.hpp"
# include "eyeline/utilx/PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPPMessageHandlersFactory : public io_subsystem::MessageHandlersFactory {
public:
  SMPPMessageHandlersFactory()
  : _logger(smsc::logger::Logger::getInstance("smpp"))
  {}

  virtual ~SMPPMessageHandlersFactory() {}

  virtual io_subsystem::MessageHandler* createMessageHandler(const io_subsystem::Packet* packet);
  virtual uint8_t getProtocolFamily() const { return SMPP_PROTOCOL; }

protected:
  template <class MESSAGE, class MESSAGE_HANDLERS>
  io_subsystem::MessageHandler*
  instantiateMessageHandler(const io_subsystem::Packet* packet) {
    utilx::alloc_mem_desc_t *ptrDescForMessage =
      utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_MESSAGE>();

    if ( ptrDescForMessage->allocated_memory_size < packet->packet_data_len )
      throw smsc::util::Exception("SMPPMessageHandlersFactory ::instantiateMessageHandler::: too small preallocated memory buffer (mem buf size=%d, packet size=%d)",
                                  ptrDescForMessage->allocated_memory_size, packet->packet_data_len);
    MESSAGE* message = new (ptrDescForMessage->allocated_memory) MESSAGE();
    message->deserialize(packet);

    utilx::alloc_mem_desc_t *ptrDescForHandler =
      utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_HANDLER>();

    if ( ptrDescForHandler->allocated_memory_size < sizeof(SMPPMessageHandler<MESSAGE, MESSAGE_HANDLERS>) )
      throw smsc::util::Exception("SMPPMessageHandlersFactory ::instantiateMessageHandler::: too small preallocated memory buffer for message handler",
                                  ptrDescForMessage->allocated_memory_size);

    return
      new (ptrDescForHandler->allocated_memory) SMPPMessageHandler<MESSAGE, MESSAGE_HANDLERS>(*message);
  }
private:
  smsc::logger::Logger* _logger;
};

}}}}

#include <eyeline/utilx/PreallocatedMemoryManager.hpp>

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_HANDLER>() {
  static const int MAX_HANDLER_SIZE=128;
  static TSD_Init_Helper<MEM_FOR_HANDLER> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(MAX_HANDLER_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
