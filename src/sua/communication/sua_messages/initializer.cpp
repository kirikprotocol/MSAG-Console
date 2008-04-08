#include "initializer.hpp"
#include "ActiveAckMessage.hpp"
#include "ActiveMessage.hpp"
#include "CLDTMessage.hpp"
#include "DAUDMessage.hpp"
#include "DAVAMessage.hpp"
#include "DRSTMessage.hpp"
#include "DUNAMessage.hpp"
#include "DUPUMessage.hpp"
#include "DownAckMessage.hpp"
#include "DownMessage.hpp"
#include "ErrorMessage.hpp"
#include "InactiveAckMessage.hpp"
#include "InactiveMessage.hpp"
#include "NotifyMessage.hpp"
#include "SCONMessage.hpp"
#include "UPAckMessage.hpp"
#include "UPMessage.hpp"
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

namespace sua_messages {

bool
initialize ()
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_msgs");
  ActiveAckMessage activeAckMessage;
  SUAMessage::registerMessageCode(activeAckMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(activeAckMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<ActiveAckMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", activeAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(activeAckMessage.getMsgCode()));

  ActiveMessage activeMessage;
  SUAMessage::registerMessageCode(activeMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(activeMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<ActiveMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", activeMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(activeMessage.getMsgCode()));

  CLDTMessage cldtMessage;
  SUAMessage::registerMessageCode(cldtMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(cldtMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<CLDTMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", cldtMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(cldtMessage.getMsgCode()));

  DAUDMessage daudMessage;
  SUAMessage::registerMessageCode(daudMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(daudMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DAUDMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", daudMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(daudMessage.getMsgCode()));

  DAVAMessage davaMessage;
  SUAMessage::registerMessageCode(davaMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(davaMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DAVAMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", davaMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(davaMessage.getMsgCode()));

  DRSTMessage drstMessage;
  SUAMessage::registerMessageCode(drstMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(drstMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DRSTMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", drstMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(drstMessage.getMsgCode()));

  DUNAMessage dunaMessage;
  SUAMessage::registerMessageCode(dunaMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(dunaMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DUNAMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", dunaMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(dunaMessage.getMsgCode()));

  DUPUMessage dupuMessage;
  SUAMessage::registerMessageCode(dupuMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(dupuMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DUPUMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", dupuMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(dupuMessage.getMsgCode()));

  DownAckMessage downAckMessage;
  SUAMessage::registerMessageCode(downAckMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(downAckMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DownAckMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", downAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(downAckMessage.getMsgCode()));

  DownMessage downMessage;
  SUAMessage::registerMessageCode(downMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(downMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<DownMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", downMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(downMessage.getMsgCode()));

  ErrorMessage errorMessage;
  SUAMessage::registerMessageCode(errorMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(errorMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<ErrorMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", errorMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(errorMessage.getMsgCode()));

  InactiveAckMessage inactiveAckMessage;
  SUAMessage::registerMessageCode(inactiveAckMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(inactiveAckMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<InactiveAckMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", inactiveAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(inactiveAckMessage.getMsgCode()));

  InactiveMessage inactiveMessage;
  SUAMessage::registerMessageCode(inactiveMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(inactiveMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<InactiveMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", inactiveMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(inactiveMessage.getMsgCode()));

  NotifyMessage notifyMessage;
  SUAMessage::registerMessageCode(notifyMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(notifyMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<NotifyMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", notifyMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(notifyMessage.getMsgCode()));

  SCONMessage sconMessage;
  SUAMessage::registerMessageCode(sconMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(sconMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<SCONMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", sconMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(sconMessage.getMsgCode()));

  UPAckMessage upAckMessage;
  SUAMessage::registerMessageCode(upAckMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(upAckMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<UPAckMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", upAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(upAckMessage.getMsgCode()));

  UPMessage upMessage;
  SUAMessage::registerMessageCode(upMessage.getMsgCode());
  io_dispatcher::SuaLayerMessagesFactory::getInstance().registerMessageCreator(SUAMessage::getMessageIndex(upMessage.getMsgCode()), new io_dispatcher::SuaLayerMessagesInstancer<UPMessage>());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", upMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(upMessage.getMsgCode()));

  return true;
}

}
