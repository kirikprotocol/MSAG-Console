#include <logger/Logger.h>

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

namespace sua_messages {

bool
initialize ()
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_msgs");
  ActiveAckMessage activeAckMessage;
  SUAMessage::registerMessageCode(activeAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", activeAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(activeAckMessage.getMsgCode()));

  ActiveMessage activeMessage;
  SUAMessage::registerMessageCode(activeMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", activeMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(activeMessage.getMsgCode()));

  CLDTMessage cldtMessage;
  SUAMessage::registerMessageCode(cldtMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", cldtMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(cldtMessage.getMsgCode()));

  DAUDMessage daudMessage;
  SUAMessage::registerMessageCode(daudMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", daudMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(daudMessage.getMsgCode()));

  DAVAMessage davaMessage;
  SUAMessage::registerMessageCode(davaMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", davaMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(davaMessage.getMsgCode()));

  DRSTMessage drstMessage;
  SUAMessage::registerMessageCode(drstMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", drstMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(drstMessage.getMsgCode()));

  DUNAMessage dunaMessage;
  SUAMessage::registerMessageCode(dunaMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", dunaMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(dunaMessage.getMsgCode()));

  DUPUMessage dupuMessage;
  SUAMessage::registerMessageCode(dupuMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", dupuMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(dupuMessage.getMsgCode()));

  DownAckMessage downAckMessage;
  SUAMessage::registerMessageCode(downAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", downAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(downAckMessage.getMsgCode()));

  DownMessage downMessage;
  SUAMessage::registerMessageCode(downMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", downMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(downMessage.getMsgCode()));

  ErrorMessage errorMessage;
  SUAMessage::registerMessageCode(errorMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", errorMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(errorMessage.getMsgCode()));

  InactiveAckMessage inactiveAckMessage;
  SUAMessage::registerMessageCode(inactiveAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", inactiveAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(inactiveAckMessage.getMsgCode()));

  InactiveMessage inactiveMessage;
  SUAMessage::registerMessageCode(inactiveMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", inactiveMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(inactiveMessage.getMsgCode()));

  NotifyMessage notifyMessage;
  SUAMessage::registerMessageCode(notifyMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", notifyMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(notifyMessage.getMsgCode()));

  SCONMessage sconMessage;
  SUAMessage::registerMessageCode(sconMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", sconMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(sconMessage.getMsgCode()));

  UPAckMessage upAckMessage;
  SUAMessage::registerMessageCode(upAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", upAckMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(upAckMessage.getMsgCode()));

  UPMessage upMessage;
  SUAMessage::registerMessageCode(upMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", upMessage.getMsgCodeTextDescription(), SUAMessage::getMessageIndex(upMessage.getMsgCode()));

  return true;
}

}
