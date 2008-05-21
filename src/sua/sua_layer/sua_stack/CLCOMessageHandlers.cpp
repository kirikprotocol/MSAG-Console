#include <sua/communication/types.hpp>
#include <sua/communication/LinkId.hpp>
#include <sua/sua_layer/messages_router/MessagesRouter.hpp>
#include <sua/sua_layer/messages_router/RoutingKey.hpp>
#include <sua/communication/libsua_messages/N_UNITDATA_IND_Message.hpp>
#include <sua/communication/libsua_messages/N_NOTICE_IND_Message.hpp>
#include <sua/communication/sua_messages/CLDRMessage.hpp>

#include "CLCOMessageHandlers.hpp"

namespace sua_stack {

CLCOMessageHandlers*
utilx::Singleton<CLCOMessageHandlers>::_instance;

CLCOMessageHandlers::CLCOMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance())
{}

void
CLCOMessageHandlers::handle(const sua_messages::CLDTMessage& cldtMessage, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDTMessage=[%s]", cldtMessage.toString().c_str());
  messages_router::RoutingKey routingKey;
  routingKey.setIncomingLinkId(linkId);

  const sua_messages::TLV_GlobalTitle& tlvGT = cldtMessage.getDestinationAddress().getGlobalTitle();
  const sua_messages::GlobalTitle& gt = tlvGT.getGlobalTitle();
  const std::string& gtStringValue = gt.getGlobalTitleDigits();
  routingKey.setDestinationGT(gtStringValue);

  bool callTerminationIndicator;
  communication::LinkId outLinkId = messages_router::MessagesRouter::getInstance().getOutLink(routingKey, &callTerminationIndicator);
  if ( callTerminationIndicator ) {
    libsua_messages::N_UNITDATA_IND_Message messageForSending(cldtMessage);
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: send N_UNITDATA_IND_Message [%s] to link with linkid=[%s]", messageForSending.toString().c_str(), outLinkId.getValue().c_str());
    _cMgr.send(outLinkId, messageForSending);
  } else {
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: forward original CLDT message=[%s] to next SGP over link with linkid=[%s]", cldtMessage.toString().c_str(), linkId.getValue().c_str());
    const sua_messages::TLV_SS7HopCount& ss7HopCount = cldtMessage.getSS7HopCount();
    if ( ss7HopCount.isSetValue() ) { 
      uint8_t hopCountValue = ss7HopCount.getHopCountValue();
      if ( --hopCountValue == 0 ) {
        smsc_log_info(_logger, "CLCOMessageHandlers::handle::: hop count has been decremented to 0, throw away message =[%s]", cldtMessage.toString().c_str());
        sua_messages::CLDRMessage cldrMessage(cldtMessage, communication::HOP_COUNTER_VIOLATION);
        _cMgr.send(linkId, cldrMessage);
        return;
      } else
        _cMgr.send(linkId, cldtMessage);
    } else
      _cMgr.send(linkId, cldtMessage);
  }
}

void
CLCOMessageHandlers::handle(const sua_messages::CLDRMessage& cldrMessage, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDRMessage=[%s]", cldrMessage.toString().c_str());

  communication::LinkId linkIdToUser;

  messages_router::RoutingKey routingKey;
  routingKey.setIncomingLinkId(linkId);

  const sua_messages::TLV_GlobalTitle& tlvGT = cldrMessage.getDestinationAddress().getGlobalTitle();
  const sua_messages::GlobalTitle& gt = tlvGT.getGlobalTitle();
  const std::string& gtStringValue = gt.getGlobalTitleDigits();
  routingKey.setDestinationGT(gtStringValue);

  bool callTerminationIndicator;
  communication::LinkId outLinkId = messages_router::MessagesRouter::getInstance().getOutLink(routingKey, &callTerminationIndicator);
  if ( callTerminationIndicator ) {
    libsua_messages::N_NOTICE_IND_Message messageForSending(cldrMessage);
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: send N_NOTICE_IND_Message [%s] to link with linkid=[%s]", messageForSending.toString().c_str(), outLinkId.getValue().c_str());
    _cMgr.send(outLinkId, messageForSending);
  } else
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: drop N_NOTICE_IND_Message");
}

}
