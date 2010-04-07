#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGESINSTANCER_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGESINSTANCER_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/MemoryAllocator.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

template <class PROTMESSAGE>
class MessagesInstancer : public utilx::MessageCreator<messages::SCCPMessage> {
public:
  virtual messages::SCCPMessage* instance() {
    utilx::alloc_mem_desc_t* mem_header =
        utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_SCCP_MESSAGE>();
    return new (mem_header->allocated_memory) PROTMESSAGE();
  }
};

}}}}

#endif
