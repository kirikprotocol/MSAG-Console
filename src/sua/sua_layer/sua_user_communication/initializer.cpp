#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>
#include "MessagesHandlingDispatcher.hpp"

namespace io_dispatcher {

template <>
class SuaLayerMessagesInstancer<sua_user_communication::EncapsulatedSuaMessage_HandlingDispatcher> : public utilx::MessageCreator<communication::Message> {
public:
  SuaLayerMessagesInstancer(uint32_t msgCode)
    : _msgCode(msgCode) {}

  virtual communication::Message* instance() {
    utilx::alloc_mem_desc_t* mem_header = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_ENCAPSULATED_MESSAGE>();
    return new (mem_header->allocated_memory) sua_user_communication::EncapsulatedSuaMessage_HandlingDispatcher(_msgCode);
  }
private:
  uint32_t _msgCode;
};

}

namespace sua_user_communication {

bool registerMessageCreators()
{
  libsua_messages::BindMessage bindMessage;
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(libsua_messages::LibsuaMessage::getMessageIndex(bindMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<BindMessage_HandlingDispatcher>());

  for(uint32_t msgCode=libsua_messages::EncapsulatedSuaMessage::_MIN_MSG_CODE;
      msgCode <= libsua_messages::EncapsulatedSuaMessage::_MAX_MSG_CODE;
      ++msgCode) {
    io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(libsua_messages::LibsuaMessage::getMessageIndex(msgCode), new io_dispatcher::SuaLayerMessagesInstancer<EncapsulatedSuaMessage_HandlingDispatcher>(msgCode));
  }

  libsua_messages::MErrorMessage mErrorMessage;
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(libsua_messages::LibsuaMessage::getMessageIndex(mErrorMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<MErrorMessage_HandlingDispatcher>());

  libsua_messages::UnbindMessage unbindMessage;
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(libsua_messages::LibsuaMessage::getMessageIndex(unbindMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<UnbindMessage_HandlingDispatcher>());

  return true;
}

}
