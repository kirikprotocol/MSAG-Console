#include "logger/Logger.h"

#include "eyeline/ss7na/common/AdaptationLayer_MsgCodesIndexer.hpp"
#include "initializer.hpp"
#include "ActiveAckMessage.hpp"
#include "ActiveMessage.hpp"
#include "CLDTMessage.hpp"
#include "CLDRMessage.hpp"
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

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

bool
initialize ()
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_msgs");
  ActiveAckMessage activeAckMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(activeAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", activeAckMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(activeAckMessage.getMsgCode()));

  ActiveMessage activeMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(activeMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", activeMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(activeMessage.getMsgCode()));

  CLDTMessage cldtMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(cldtMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", cldtMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(cldtMessage.getMsgCode()));

  CLDRMessage cldrMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(cldrMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", cldrMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(cldrMessage.getMsgCode()));

  DAUDMessage daudMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(daudMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", daudMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(daudMessage.getMsgCode()));

  DAVAMessage davaMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(davaMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", davaMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(davaMessage.getMsgCode()));

  DRSTMessage drstMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(drstMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", drstMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(drstMessage.getMsgCode()));

  DUNAMessage dunaMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(dunaMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", dunaMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(dunaMessage.getMsgCode()));

  DUPUMessage dupuMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(dupuMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", dupuMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(dupuMessage.getMsgCode()));

  DownAckMessage downAckMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(downAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", downAckMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(downAckMessage.getMsgCode()));

  DownMessage downMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(downMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", downMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(downMessage.getMsgCode()));

  ErrorMessage errorMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(errorMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", errorMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(errorMessage.getMsgCode()));

  InactiveAckMessage inactiveAckMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(inactiveAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", inactiveAckMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(inactiveAckMessage.getMsgCode()));

  InactiveMessage inactiveMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(inactiveMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", inactiveMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(inactiveMessage.getMsgCode()));

  NotifyMessage notifyMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(notifyMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", notifyMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(notifyMessage.getMsgCode()));

  SCONMessage sconMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(sconMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", sconMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(sconMessage.getMsgCode()));

  UPAckMessage upAckMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(upAckMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", upAckMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(upAckMessage.getMsgCode()));

  UPMessage upMessage;
  common::AdaptationLayer_MsgCodesIndexer::registerMessageCode(upMessage.getMsgCode());
  smsc_log_info(logger, "sua_messages::initialize::: for message [%s] assigned message index=[%d]", upMessage.getMsgCodeTextDescription(), common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(upMessage.getMsgCode()));

  return true;
}

}}}}}
