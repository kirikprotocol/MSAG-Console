#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

#include "eyeline/corex/io/IOExceptions.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/MTP3SapInfo.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/MTP3SapSelectPolicy.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/PolicyRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/router/GTTranslator.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/ReassemblyProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

template <class MSG>
void
MessageHandlers::handleRouteOnGT(MSG& message)
{
  const eyeline::sccp::SCCPAddress& calledAddr = message.getCalledAddress();
  router::GTTranslationResult trRes =
      router::GTTranslator::getInstance().translate(calledAddr);

  if ( trRes.routeToMTP3 ) {
    if ( decrementHopCounterAndCheckItViolated(message) == HOP_COUNTER_VIOLATED )
      throw common::SCCPException(common::HOP_COUNTER_VIOLATION, "MessageHandlers::handleRouteOnGT::: hop counter violated");

    forwardMessageToMTP3(trRes.routeId, message);
  } else
    forwardMessageToApplication(trRes.routeId, message);
}

template <class MSG>
inline
void
MessageHandlers::forwardMessageToApplication(const std::string& route_id,
                                             const MSG& message)
{
  smsc_log_debug(_logger, "MessageHandlers::forwardMessageToApplication::: forward message='%s' to route with id='%s'",
                 message.toString().c_str(), route_id.c_str());
  SCCPUserSelectPolicyRefPtr sccpUserPolicy =
      PolicyRegistry<SCCPUserSelectPolicy>::getInstance().lookup(route_id);

  std::string appId = sccpUserPolicy->getValue();
  std::string firstAppId = appId;
  const libsccp::LibsccpMessage* indicationMsg = createIndicationMessage(message);

  do {
    common::LinkId outLinkId;
    try {
      const eyeline::sccp::SCCPAddress& calledAddr = message.getCalledAddress();
      if ( calledAddr.getIndicator().hasSSN && calledAddr.getSSN() > 0 )
        outLinkId = common::sccp_sap::ActiveAppsRegistry::getInstance().getLink(appId, calledAddr.getSSN());
      else
        outLinkId = common::sccp_sap::ActiveAppsRegistry::getInstance().getLink(appId);

      _cMgr.send(outLinkId, *indicationMsg);
      return;
    } catch (common::RouteNotFound& ex) {
      smsc_log_error(_logger, "MessageHandlers::forwardMessageToApplication::: caught exception RouteNotFound='%s', communication failure to application='%s'. Try next application if any",
                     ex.what(), appId.c_str());
      common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkId);
    } catch (common::io_dispatcher::ProtocolException& ex) {
      smsc_log_error(_logger, "MessageHandlers::forwardMessageToApplication::: caught ProtocolException='%s', communication failure to application='%s'. Try next application if any",
                     ex.what(), appId.c_str());
      common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkId);
    } catch (corex::io::BrokenPipe& ex) {
      smsc_log_error(_logger, "MessageHandlers::forwardMessageToApplication::: caught exception BrokenPipe='%s', communication failure to application='%s'. Try next application if any",
                     ex.what(), appId.c_str());
      common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkId);
    }

    appId = sccpUserPolicy->getValue();
  } while(appId != firstAppId);
  throw common::TranslationFailure(common::SCCP_FAILURE, "MessageHandlers::forwardMessageToApplication::: no one SCCP user is accessible");
}

template <>
inline
void
MessageHandlers::forwardMessageToApplication<messages::XUDT>(const std::string& route_id,
                                                             const messages::XUDT& message)
{
  smsc_log_debug(_logger, "MessageHandlers::forwardMessageToApplication::: forward XUDT message='%s' to route with id='%s'",
                 message.toString().c_str(), route_id.c_str());
  reassembleAndSend(route_id, message);
}

template <>
inline
void
MessageHandlers::forwardMessageToApplication<messages::XUDTS>(const std::string& route_id,
                                                             const messages::XUDTS& message)
{
  smsc_log_debug(_logger, "MessageHandlers::forwardMessageToApplication::: forward XUDTS message='%s' to route with id='%s'",
                 message.toString().c_str(), route_id.c_str());
  reassembleAndSend(route_id, message);
}

template <class MSG>
inline
void
MessageHandlers::reassembleAndSend(const std::string& route_id,
                                   const MSG& message)
{
  SCCPUserSelectPolicyRefPtr sccpUserPolicy =
      PolicyRegistry<SCCPUserSelectPolicy>::getInstance().lookup(route_id);

  std::string appId = sccpUserPolicy->getValue();
  std::string firstAppId = appId;

  uint8_t assembledMessage[4096];
  uint16_t assembledMessageSz=0;
  const libsccp::LibsccpMessage* indicationMsg;
  if ( needReassembling(message) ) {
    if ( ReassemblyProcessor::getInstance().reassemble(message, assembledMessage,
                                                     &assembledMessageSz,
                                                     sizeof(assembledMessage)) != ReassemblyProcessor::FullMessageReassembled )
      return;
    indicationMsg = createIndicationMessage(message, assembledMessage, assembledMessageSz);
  } else
    indicationMsg = createIndicationMessage(message);

  do {
    common::LinkId outLinkId;
    try{
      const eyeline::sccp::SCCPAddress& calledAddr = message.getCalledAddress();
      if ( calledAddr.getIndicator().hasSSN && calledAddr.getSSN() > 0 )
        outLinkId = common::sccp_sap::ActiveAppsRegistry::getInstance().getLink(appId, calledAddr.getSSN());
      else
        outLinkId = common::sccp_sap::ActiveAppsRegistry::getInstance().getLink(appId);

      _cMgr.send(outLinkId, *indicationMsg);
      return;
    } catch (common::RouteNotFound& ex) {
      smsc_log_error(_logger, "MessageHandlers::reassembleAndSend::: application='%s' is inactive. Try next application if any",
                           appId.c_str());
      appId = sccpUserPolicy->getValue();
    } catch (corex::io::BrokenPipe& ex) {
      smsc_log_error(_logger, "MessageHandlers::reassembleAndSend::: caught exception BrokenPipe='%s', communication failure to application='%s'. Try next application if any",
                     ex.what(), appId.c_str());
      common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkId);
    }
  } while(appId != firstAppId);
  throw common::TranslationFailure(common::SCCP_FAILURE, "MessageHandlers::reassembleAndSend::: no one SCCP user is accessible");
}

template <class MSG>
inline
void
MessageHandlers::handleRouteOnSSN(const MSG& message)
{
  const eyeline::sccp::SCCPAddress& calledAddr = message.getCalledAddress();
  if ( !calledAddr.getIndicator().hasSSN )
    throw common::RouteNotFound(common::SUBSYSTEM_FAILURE, "MessageHandlers::handleRouteOnSSN::: invalid called address value - set route on SSN flag and SSN is absent");

  common::LinkId& outLinkId = common::sccp_sap::ActiveAppsRegistry::getInstance().getLink(calledAddr.getSSN());
  try {
    _cMgr.send(outLinkId, *createIndicationMessage(message));
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "MessageHandlers::reassembleAndSend::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), outLinkId.getValue().c_str());
    common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkId);
  }
}

template <>
inline
void
MessageHandlers::handleRouteOnSSN<messages::XUDT>(const messages::XUDT& message)
{
  const eyeline::sccp::SCCPAddress& calledAddr = message.getCalledAddress();
  if ( !calledAddr.getIndicator().hasSSN )
    throw common::RouteNotFound(common::SUBSYSTEM_FAILURE, "MessageHandlers::handleRouteOnSSN::: invalid called address value - set route on SSN flag and SSN is absent");

  uint8_t assembledMessage[4096];
  uint16_t assembledMessageSz=0;
  if ( needReassembling(message) &&
       ReassemblyProcessor::getInstance().reassemble(message, assembledMessage,
                                                     &assembledMessageSz,
                                                     sizeof(assembledMessage)) != ReassemblyProcessor::FullMessageReassembled )
    return;

  common::LinkId& outLinkId = common::sccp_sap::ActiveAppsRegistry::getInstance().getLink(calledAddr.getSSN());
  try {
    if ( assembledMessageSz )
      _cMgr.send(outLinkId, *createIndicationMessage(message, assembledMessage, assembledMessageSz));
    else
      _cMgr.send(outLinkId, *createIndicationMessage(message));
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "MessageHandlers::handleRouteOnSSN<messages::XUDT>::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), outLinkId.getValue().c_str());
    common::sccp_sap::ActiveAppsRegistry::getInstance().remove(outLinkId);
  }
}

template <class MSG>
inline
MessageHandlers::hop_counter_check_res_e
MessageHandlers::decrementHopCounterAndCheckItViolated(MSG& mssage)
{
  uint8_t hopCounter = mssage.getHopCounter();
  if ( --hopCounter == 0 )
    return HOP_COUNTER_VIOLATED;

  mssage.setHopCounter(hopCounter);
  return HOP_COUNTER_CHECK_OK;
}

template <>
inline
MessageHandlers::hop_counter_check_res_e
MessageHandlers::decrementHopCounterAndCheckItViolated<messages::UDT>(messages::UDT& mssage)
{
  return HOP_COUNTER_CHECK_OK;
}

template <>
inline
MessageHandlers::hop_counter_check_res_e
MessageHandlers::decrementHopCounterAndCheckItViolated<messages::UDTS>(messages::UDTS& mssage)
{
  return HOP_COUNTER_CHECK_OK;
}

}}}}
