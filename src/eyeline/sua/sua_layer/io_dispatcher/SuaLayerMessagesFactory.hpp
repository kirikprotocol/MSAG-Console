#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_SUALAYERMESSAGESFACTORY_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_SUALAYERMESSAGESFACTORY_HPP__

# include <eyeline/utilx/PreallocatedMemoryManager.hpp>
# include <eyeline/utilx/MessagesFactory.hpp>
# include <eyeline/utilx/Singleton.hpp>

# include <eyeline/sua/communication/Message.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
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

}}}}

#endif
