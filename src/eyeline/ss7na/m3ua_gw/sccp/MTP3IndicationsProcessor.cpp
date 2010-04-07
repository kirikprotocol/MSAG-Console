#include <list>
#include "MTP3IndicationsProcessor.hpp"
#include "MessageHandlingDispatcherIface.hpp"
#include "MessagesFactory.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"

#include "eyeline/ss7na/m3ua_gw/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/types.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/PointsDefinitionRegistry.hpp"
#include "eyeline/ss7na/libsccp/messages/N_PCSTATE_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

void
MTP3IndicationsProcessor::handle(const mtp3::primitives::MTP_Pause_Ind& mtp_primitive)
{
  // TODO: Discontinues all subsystem status tests
  if ( !mtp3::PointsDefinitionRegistry::getInstance().exists(mtp_primitive.getAffectedDPC()) ) {
    smsc_log_debug(_logger, "MTP3IndicationsProcessor::handle::: handle primitive MTP_Pause_Ind=[%s]",
                   mtp_primitive.toString().c_str());

    libsccp::N_PCSTATE_IND_Message n_pcstate;
    n_pcstate.setAffectedSignalingPoint(mtp_primitive.getAffectedDPC());
    n_pcstate.setSignalingPointStatus(libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_INACCESSIBLE);

    broadcastIndication(n_pcstate);
  }
}

void
MTP3IndicationsProcessor::handle(const mtp3::primitives::MTP_Resume_Ind& mtp_primitive)
{
  if ( !mtp3::PointsDefinitionRegistry::getInstance().exists(mtp_primitive.getAffectedDPC()) ) {
    smsc_log_debug(_logger, "MTP3IndicationsProcessor::handle::: handle primitive MTP_Resume_Ind");

    libsccp::N_PCSTATE_IND_Message n_pcstate;
    n_pcstate.setAffectedSignalingPoint(mtp_primitive.getAffectedDPC());
    n_pcstate.setSignalingPointStatus(libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_ACCESSIBLE);

    broadcastIndication(n_pcstate);
  }
}

void
MTP3IndicationsProcessor::handle(const mtp3::primitives::MTP_Status_Ind& mtp_primitive)
{
  if ( !mtp3::PointsDefinitionRegistry::getInstance().exists(mtp_primitive.getAffectedDPC()) ) {
    smsc_log_debug(_logger, "MTP3IndicationsProcessor::handle::: handle primitive MTP_Status_Ind=[%s]",
                   mtp_primitive.toString().c_str());
    if ( mtp_primitive.getCause() == InaccessibleRemoteUser ||
        mtp_primitive.getCause() == UnequippedRemoteUser ) {
      libsccp::N_PCSTATE_IND_Message n_pcstate;
      n_pcstate.setAffectedSignalingPoint(mtp_primitive.getAffectedDPC());
      n_pcstate.setSignalingPointStatus(libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_INACCESSIBLE);

      broadcastIndication(n_pcstate);
    } else if ( mtp_primitive.getCause() == SignallingNetworkCongestion  ) {
      libsccp::N_PCSTATE_IND_Message n_pcstate;
      n_pcstate.setAffectedSignalingPoint(mtp_primitive.getAffectedDPC());
      n_pcstate.setSignalingPointStatus(libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_CONGESTED);

      broadcastIndication(n_pcstate);
    }
  }
}

void
MTP3IndicationsProcessor::handle(const mtp3::primitives::MTP_Transfer_Ind& mtp_primitive)
{
  smsc_log_info(_logger, "MTP3IndicationsProcessor::handle::: handle primitive MTP_Transfer_Ind=[%s]",
                mtp_primitive.toString().c_str());
  try {
    const uint8_t* sccpData = mtp_primitive.getData().data;
    uint16_t sccpDataLen = mtp_primitive.getData().dataLen;
    messages::SCCPMessage* sccpMessage = MessagesFactory::getInstance().instanceMessage(*sccpData);
    if ( sccpMessage ) {
      common::TP tp(0, sccpDataLen, const_cast<uint8_t*>(sccpData), sccpDataLen);
      sccpMessage->deserialize(tp);
      MessageHandlingDispatcherIface* msgHandleDisp = sccpMessage->getHandlingDispatcher();
      if ( !msgHandleDisp ) {
        smsc_log_error(_logger, "MTP3IndicationsProcessor::handle::: fatal error - message handler isn't set for message type=%u",
                       sccpMessage->getMsgCode());
        return;
      }
      sccpMessage->setSLS(mtp_primitive.getSLS());
      msgHandleDisp->dispatch_handle();
    } else
      smsc_log_error(_logger, "MTP3IndicationsProcessor::handle::: can't instantiate SCCP message for msg_code [=%d]", *sccpData);
  } catch (SCCPFailureException& ex) {
    smsc_log_error(_logger, "MTP3IndicationsProcessor::handle::: caught SCCPFailureException");
    formMtpTransferReq(mtp_primitive, ex.getResponseMsg());
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MTP3IndicationsProcessor::handle::: caught Exception='%s'", ex.what());
  }
}

void
MTP3IndicationsProcessor::formMtpTransferReq(const mtp3::primitives::MTP_Transfer_Ind& mtp_ind_primitive,
                                             const messages::SCCPMessage& negative_message)
{
  smsc_log_debug(_logger, "MTP3IndicationsProcessor::formMtpTransferReq::: form mtp3 transfer request primitive to send negative sccp message='%s'",
                 negative_message.toString().c_str());
  try {
    common::TP tp;
    negative_message.serialize(&tp);
    mtp3::primitives::MTP_Transfer_Req mtpPrimitive(mtp_ind_primitive.getDPC(),
                                                    mtp_ind_primitive.getOPC(),
                                                    mtp_ind_primitive.getSLS(),
                                                    mtp_ind_primitive.getServiceIndicator(),
                                                    mtp_ind_primitive.getNetworkIndicator(),
                                                    mtp_ind_primitive.getMsgPrio(),
                                                    utilx::variable_data_t(tp.packetBody, static_cast<uint16_t>(tp.packetLen)));

    mtp3::sap::MTPSapInstance::getInstance().handle(mtpPrimitive);
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MTP3IndicationsProcessor::formMtpTransferReq::: cajght exception='%s'",
                   ex.what());
  }
}

void
MTP3IndicationsProcessor::broadcastIndication(libsccp::LibsccpMessage& libsccp_msg)
{
  std::list<common::LinkId> activeApps;
  common::sccp_sap::ActiveAppsRegistry::getInstance().getAllActiveAppLinks(&activeApps);
  for(std::list<common::LinkId>::const_iterator iter = activeApps.begin(), end_iter = activeApps.end();
      iter != end_iter; ++iter) {
    try {
      _cMgr.send(*iter, libsccp_msg);
    } catch (corex::io::BrokenPipe& ex) {
      smsc_log_error(_logger, "MTP3IndicationsProcessor::broadcastIndication::: caught exception BrokenPipe='%s', communication failure to link='%s'",
                     ex.what(), (*iter).getValue().c_str());
      common::sccp_sap::ActiveAppsRegistry::getInstance().remove(*iter);
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "MTP3IndicationsProcessor::broadcastIndication::: caught unexpected exception='%s'",
                     ex.what());
    }
  }
}

}}}}
