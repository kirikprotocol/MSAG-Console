#include "initializer.hpp"
#include "EncapsulatedSuaMessage.hpp"
#include "BindMessage.hpp"
#include "UnbindMessage.hpp"
#include "MErrorMessage.hpp"

#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

namespace io_dispatcher {

template <>
class SuaLayerMessagesInstancer<libsua_messages::EncapsulatedSuaMessage> : public utilx::MessageCreator<communication::Message> {
public:
  SuaLayerMessagesInstancer(uint32_t msgCode)
    : _msgCode(msgCode) {}

  virtual communication::Message* instance() {
    utilx::alloc_mem_desc_t* mem_header = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_ENCAPSULATED_MESSAGE>();
    return new (mem_header->allocated_memory) libsua_messages::EncapsulatedSuaMessage(_msgCode);
  }
private:
  uint32_t _msgCode;
};

}

namespace libsua_messages {

bool
initialize ()
{
  BindMessage bindMessage;
  LibsuaMessage::registerMessageCode(bindMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(LibsuaMessage::getMessageIndex(bindMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<BindMessage>());

  for(uint32_t msgCode=EncapsulatedSuaMessage::_MIN_MSG_CODE; msgCode <= EncapsulatedSuaMessage::_MAX_MSG_CODE; ++msgCode) {
    LibsuaMessage::registerMessageCode(msgCode);
    io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(LibsuaMessage::getMessageIndex(msgCode), new io_dispatcher::SuaLayerMessagesInstancer<EncapsulatedSuaMessage>(msgCode));
  }

  MErrorMessage mErrorMessage;
  LibsuaMessage::registerMessageCode(mErrorMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(LibsuaMessage::getMessageIndex(mErrorMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<MErrorMessage>());

   UnbindMessage unbindMessage;
  LibsuaMessage::registerMessageCode(unbindMessage.getMsgCode());

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(LibsuaMessage::getMessageIndex(unbindMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<UnbindMessage>());
  
  return true;
}

}
