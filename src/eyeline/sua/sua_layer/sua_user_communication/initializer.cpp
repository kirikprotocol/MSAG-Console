#include <eyeline/sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>
#include "MessagesHandlingDispatcher.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

bool registerMessageCreators()
{
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     communication::libsua_messages::LibsuaMessage::getMessageIndex(communication::libsua_messages::BindMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<BindMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     communication::libsua_messages::LibsuaMessage::getMessageIndex(communication::libsua_messages::N_UNITDATA_REQ_Message().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<N_UNITDATA_REQ_Message_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     communication::libsua_messages::LibsuaMessage::getMessageIndex(communication::libsua_messages::UnbindMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<UnbindMessage_HandlingDispatcher>()
     );
  return true;
}

}}}}
