#include <sua/communication/types.hpp>
#include <sua/communication/LinkId.hpp>

#include <sua/communication/libsua_messages/N_UNITDATA_IND_Message.hpp>
#include <sua/communication/libsua_messages/N_NOTICE_IND_Message.hpp>
#include <sua/communication/sua_messages/CLDRMessage.hpp>

#include <sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>
#include <sua/sua_layer/messages_router/GTTranslator.hpp>

#include "CLCOMessageHandlers.hpp"

namespace sua_stack {

CLCOMessageHandlers::CLCOMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance())
{}

void
CLCOMessageHandlers::prepareAndSendCLDRMessage(const sua_messages::CLDTMessage& cldtMessage,
                                               const communication::LinkId& linkId,
                                               communication::return_cause_value_t returnCause)
{
  const sua_messages::ProtocolClass& protocolClassValue = cldtMessage.getProtocolClass().getProtocolClassValue();
  if ( protocolClassValue.getRetOnErrorIndication() ) {
    sua_messages::CLDRMessage cldrMessage(cldtMessage, returnCause);
    _cMgr.send(linkId, cldrMessage);
  }
}

void
CLCOMessageHandlers::handle(const sua_messages::CLDTMessage& cldtMessage,
                            const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDTMessage=[%s]", cldtMessage.toString().c_str());

  try {
    const communication::LinkId& outLinkSetId = messages_router::GTTranslator::getInstance().translate(cldtMessage.getDestinationAddress());

    std::string appId;
    if ( sua_user_communication::LinkSetInfoRegistry::getInstance().getAppId(outLinkSetId, &appId) ) {
      libsua_messages::N_UNITDATA_IND_Message messageForSending(cldtMessage);
      smsc_log_info(_logger, "CLCOMessageHandlers::handle::: send N_UNITDATA_IND_Message [%s] to link with linkid=[%s]", messageForSending.toString().c_str(), outLinkSetId.getValue().c_str());
      _cMgr.send(outLinkSetId, messageForSending);
    } else {
      smsc_log_info(_logger, "CLCOMessageHandlers::handle::: forward original CLDT message=[%s] to next SGP over link with linkid=[%s]", cldtMessage.toString().c_str(), linkId.getValue().c_str());
      const sua_messages::TLV_SS7HopCount& ss7HopCount = cldtMessage.getSS7HopCount();
      if ( ss7HopCount.isSetValue() ) { 
        uint8_t hopCountValue = ss7HopCount.getHopCountValue();
        if ( --hopCountValue == 0 ) {
          smsc_log_info(_logger, "CLCOMessageHandlers::handle::: hop count has been decremented to 0, throw away message =[%s]", cldtMessage.toString().c_str());
          prepareAndSendCLDRMessage(cldtMessage, linkId, communication::HOP_COUNTER_VIOLATION);
          return;
        } else
          _cMgr.send(linkId, cldtMessage);
      } else
        _cMgr.send(linkId, cldtMessage);
    }
  } catch (utilx::TranslationFailure& ex) {
    prepareAndSendCLDRMessage(cldtMessage, linkId, ex.getFailureCode());
  }
}

void
CLCOMessageHandlers::handle(const sua_messages::CLDRMessage& cldrMessage,
                            const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDRMessage=[%s]", cldrMessage.toString().c_str());

  try {
    communication::LinkId outLinkSetId = messages_router::GTTranslator::getInstance().translate(cldrMessage.getDestinationAddress());

    std::string appId;
    if ( sua_user_communication::LinkSetInfoRegistry::getInstance().getAppId(outLinkSetId, &appId) ) {
      libsua_messages::N_NOTICE_IND_Message messageForSending(cldrMessage);
      smsc_log_info(_logger, "CLCOMessageHandlers::handle::: send N_NOTICE_IND_Message [%s] to link with linkid=[%s]", messageForSending.toString().c_str(), outLinkSetId.getValue().c_str());
      _cMgr.send(outLinkSetId, messageForSending);
    } else
      smsc_log_info(_logger, "CLCOMessageHandlers::handle::: drop N_NOTICE_IND_Message");
  } catch (utilx::TranslationFailure& ex) {
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: catch exception utilx::TranslationFailure [%s]", ex.what());
  }
}

}
