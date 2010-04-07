#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_MESSAGESINSTANCER_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_MESSAGESINSTANCER_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/MessagesFactory.hpp"
# include "eyeline/utilx/PreallocatedMemoryManager.hpp"

# include "eyeline/ss7na/common/Message.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

template <class PROTMESSAGE>
class MessagesInstancer : public utilx::MessageCreator<common::Message> {
public:
  virtual common::Message* instance() {
    utilx::alloc_mem_desc_t* mem_header = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_MESSAGE>();
    return new (mem_header->allocated_memory) PROTMESSAGE();
  }
};

}}}}

#endif
