#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGESINSTANCER_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGESINSTANCER_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/MemoryAllocator.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

template <class PROTMESSAGE>
class MessagesInstancer : public utilx::MessageCreator<messages::SCMGMessage> {
public:
  virtual messages::SCMGMessage* instance() {
    utilx::alloc_mem_desc_t* mem_header =
        utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_SCCP_SCMG_MESSAGE>();
    return new (mem_header->allocated_memory) PROTMESSAGE();
  }
};

}}}}}

#endif
