#include "eyeline/ss7na/common/types.hpp"
#include "eyeline/ss7na/common/LinkId.hpp"
#include "eyeline/ss7na/common/Exception.hpp"

#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/sua_gw/messages_router/GTTranslator.hpp"
#include "eyeline/ss7na/sua_gw/sccp_sap/messages/N_UNITDATA_IND_Message.hpp"
#include "eyeline/ss7na/sua_gw/sccp_sap/messages/N_NOTICE_IND_Message.hpp"
#include "eyeline/ss7na/sua_gw/sccp_sap/LinkSetInfoRegistry.hpp"
#include "messages/CLDRMessage.hpp"

#include "CLCOMessageHandlers.hpp"
#include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"
#include "messages/ProtocolClass.hpp"
#include "ReassemblyProcessor.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"

using eyeline::ss7na::libsccp::N_UNITDATA_IND_Message;

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

CLCOMessageHandlers::CLCOMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance())
{}

void
CLCOMessageHandlers::prepareAndSendCLDRMessage(const messages::CLDTMessage& cldt_message,
                                               const common::LinkId& link_id,
                                               common::return_cause_value_t return_cause)
{
  const messages::ProtocolClass& protocolClassValue = cldt_message.getProtocolClass().getProtocolClassValue();
  if ( protocolClassValue.getRetOnErrorIndication() ) {
    messages::CLDRMessage cldrMessage(cldt_message, return_cause);
    _cMgr.send(link_id, cldrMessage);
  }
}

void
CLCOMessageHandlers::handle(const messages::CLDTMessage& cldt_message,
                            const common::LinkId& link_id)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDTMessage=[%s]", cldt_message.toString().c_str());

  try {
    const common::LinkId& outLinkSetId = messages_router::GTTranslator::getInstance().translate(cldt_message.getDestinationAddress());

    std::string appId;
    if ( sccp_sap::LinkSetInfoRegistry::getInstance().getAppId(outLinkSetId, &appId) )
      sendIndicationPrimitive(cldt_message, outLinkSetId);
    else
      forwardMessageToSGP(cldt_message, link_id);
  } catch (common::TranslationFailure& ex) {
    try {
      prepareAndSendCLDRMessage(cldt_message, link_id, ex.getFailureCode());
    } catch(corex::io::BrokenPipe& ex) {
      smsc_log_error(_logger, "CLCOMessageHandlers::handle::: caught exception BrokenPipe='%s' for link ='%s'",
                     ex.what(), link_id.getValue().c_str());
      common::io_dispatcher::LinkPtr brokeConn = _cMgr.removeLink(link_id);
      if ( brokeConn.Get() )
        common::io_dispatcher::Reconnector::getInstance().schedule(brokeConn);
    }
  }
}

void
CLCOMessageHandlers::sendIndicationPrimitive(const messages::CLDTMessage& cldt_message,
                                             const common::LinkId& out_linkset_id)
{
  try {
    if ( !cldt_message.isSetSegmentation() ||
        (cldt_message.getSegmentation().isFirstSegment() && cldt_message.getSegmentation().getNumOfRemainingSegments() == 0) ) {
      sccp_sap::messages::N_UNITDATA_IND_Message messageForSending(cldt_message);
      smsc_log_info(_logger, "CLCOMessageHandlers::sendIndicationPrimitive::: send N_UNITDATA_IND_Message [%s] to link with linkid=[%s]",
                    messageForSending.toString().c_str(), out_linkset_id.getValue().c_str());
      _cMgr.send(out_linkset_id, messageForSending);
    } else {
      uint8_t totalMsg[65535];
      uint16_t realMsgSz=0;
      if (ReassemblyProcessor::getInstance().reassemble(cldt_message, totalMsg, &realMsgSz, sizeof(totalMsg)) == ReassemblyProcessor::FullMessageReassembled) {
        sccp_sap::messages::N_UNITDATA_IND_Message messageForSending(cldt_message);
        messageForSending.setUserData(totalMsg, realMsgSz);
        smsc_log_info(_logger, "CLCOMessageHandlers::sendIndicationPrimitive::: send N_UNITDATA_IND_Message [%s] to link with linkid=[%s]",
                      messageForSending.toString().c_str(), out_linkset_id.getValue().c_str());
        _cMgr.send(out_linkset_id, messageForSending);
      }
    }
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "CLCOMessageHandlers::sendIndicationPrimitive::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), out_linkset_id.getValue().c_str());
    common::sccp_sap::ActiveAppsRegistry::getInstance().remove(out_linkset_id);
  }

}

void
CLCOMessageHandlers::forwardMessageToSGP(const messages::CLDTMessage& cldt_message,
                                         const common::LinkId& link_id)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::forwardMessageToSGP::: forward original CLDT message=[%s] to next SGP over link with linkid=[%s]", cldt_message.toString().c_str(), link_id.getValue().c_str());
  const messages::TLV_SS7HopCount& ss7HopCount = cldt_message.getSS7HopCount();
  try {
    if ( ss7HopCount.isSetValue() ) {
      uint8_t hopCountValue = ss7HopCount.getHopCountValue();
      if ( --hopCountValue == 0 ) {
        smsc_log_info(_logger, "CLCOMessageHandlers::forwardMessageToSGP::: hop count has been decremented to 0, throw away message =[%s]", cldt_message.toString().c_str());
        prepareAndSendCLDRMessage(cldt_message, link_id, common::HOP_COUNTER_VIOLATION);
        return;
      } else
        _cMgr.send(link_id, cldt_message);
    } else
      _cMgr.send(link_id, cldt_message);
  } catch(corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "CLCOMessageHandlers::forwardMessageToSGP::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), link_id.getValue().c_str());
    common::io_dispatcher::LinkPtr brokeConn = _cMgr.removeLink(link_id);
    if ( brokeConn.Get() )
      common::io_dispatcher::Reconnector::getInstance().schedule(brokeConn);
  }
}

void
CLCOMessageHandlers::handle(const messages::CLDRMessage& cldr_message,
                            const common::LinkId& link_id)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDRMessage=[%s]", cldr_message.toString().c_str());

  common::LinkId outLinkSetId;
  try {
    outLinkSetId = messages_router::GTTranslator::getInstance().translate(cldr_message.getDestinationAddress());

    std::string appId;
    if ( sccp_sap::LinkSetInfoRegistry::getInstance().getAppId(outLinkSetId, &appId) ) {
      sccp_sap::messages::N_NOTICE_IND_Message messageForSending(cldr_message);
      smsc_log_info(_logger, "CLCOMessageHandlers::handle::: send N_NOTICE_IND_Message [%s] to link with linkid=[%s]", messageForSending.toString().c_str(), outLinkSetId.getValue().c_str());
      _cMgr.send(outLinkSetId, messageForSending);
    } else
      smsc_log_info(_logger, "CLCOMessageHandlers::handle::: drop N_NOTICE_IND_Message");
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "CLCOMessageHandlers::handle::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), outLinkSetId.getValue().c_str());
    common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkSetId);
  } catch (common::TranslationFailure& ex) {
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: catch exception utilx::TranslationFailure [%s]", ex.what());
  }
}

}}}}
