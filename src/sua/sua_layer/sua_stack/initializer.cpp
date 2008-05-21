#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>
#include "MessagesHandlingDispatcher.hpp"

namespace sua_stack {

bool registerMessageCreators()
{
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::ActiveAckMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<ActiveAckMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::ActiveMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<ActiveMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::CLDTMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<CLDTMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::CLDRMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<CLDRMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::DAVAMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<DAVAMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::DRSTMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<DRSTMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::DUNAMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<DUNAMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::DUPUMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<DUPUMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::DownAckMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<DownAckMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::DownMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<DownMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::ErrorMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<ErrorMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::InactiveAckMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<InactiveAckMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::InactiveMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<InactiveMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::NotifyMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<NotifyMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::SCONMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<SCONMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::UPAckMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<UPAckMessage_HandlingDispatcher>()
     );

  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator
    (
     sua_messages::SUAMessage::getMessageIndex(sua_messages::UPMessage().getMsgCode()),
     new io_dispatcher::SuaLayerMessagesInstancer<UPMessage_HandlingDispatcher>()
     );

  return true;
}

}
