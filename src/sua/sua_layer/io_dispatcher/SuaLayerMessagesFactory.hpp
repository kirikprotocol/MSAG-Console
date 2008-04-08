#ifndef __SUA_LAYER_IO_DISPATCHER_SUALAYERMESSAGESFACTORY_HPP__
# define __SUA_LAYER_IO_DISPATCHER_SUALAYERMESSAGESFACTORY_HPP__ 1

# include <sua/utilx/PreallocatedMemoryManager.hpp>
# include <sua/utilx/MessagesFactory.hpp>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/Message.hpp>

namespace io_dispatcher {

const unsigned int MAX_NUM_OF_MESSAGES = 100;

class SuaLayerMessagesFactory : public utilx::MessagesFactoryTmpl<communication::Message, MAX_NUM_OF_MESSAGES>,
                                public utilx::Singleton<SuaLayerMessagesFactory> {
public:
};

template <class PROTMESSAGE>
class SuaLayerMessagesInstancer : public utilx::MessageCreator<communication::Message> {
public:
  virtual communication::Message* instance() {
    utilx::alloc_mem_desc_t* mem_header = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_MESSAGE>();
    return new (mem_header->allocated_memory) PROTMESSAGE();
  }
};

}

#endif
