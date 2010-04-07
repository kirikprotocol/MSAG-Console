#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "eyeline/ss7na/sua_gw/sccp_sap/LinkSetInfoRegistry.hpp"
#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

SignalingNetworkManagementMessageHandlers::SignalingNetworkManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DAVAMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DAVAMessage [%s]", message.toString().c_str());

  if ( message.getSSN().isSetValue() ) {
    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsccp::N_STATE_IND_Message::USER_IN_SERVICE));
  } else {
    common::ITU_PC affectedPointCode;
    size_t offset=0;
    while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_ACCESSIBLE));
    }
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DRSTMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DRSTMessage [%s]", message.toString().c_str());
  if ( message.getSSN().isSetValue() &&
       message.getSMI().isSetValue() )
    sendSCCPManagementMessageToApplications(make_N_COORD_Indication_message(message));
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DUNAMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUNAMessage [%s]", message.toString().c_str());

  if ( message.getSSN().isSetValue() ) {
    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsccp::N_STATE_IND_Message::USER_OUT_OF_SERVICE));
  } else {
    common::ITU_PC affectedPointCode;
    size_t offset=0;
    while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_INACCESSIBLE));
    }
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DUPUMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUPUMessage [%s]", message.toString().c_str());
  common::ITU_PC affectedPointCode;
  size_t offset=0;
  while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
    sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode,
                                                                              libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_INACCESSIBLE,
                                                                              libsccp::N_PCSTATE_IND_Message::remote_sccp_status_e(message.getUserCause().getCause())));
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::SCONMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle SCONMessage [%s]", message.toString().c_str());

  if ( message.getSSN().isSetValue() ) {
    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsccp::N_STATE_IND_Message::USER_OUT_OF_SERVICE));
  } else {
    common::ITU_PC affectedPointCode;
    size_t offset=0;
    while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_CONGESTED));
    }
  }
}

void
SignalingNetworkManagementMessageHandlers::sendSCCPManagementMessageToApplications(const libsccp::LibsccpMessage& output_message)
{
  std::set<common::LinkId> linksetIds = sccp_sap::LinkSetInfoRegistry::getInstance().getLinkSetIds();
  for (std::set<common::LinkId>::const_iterator iter = linksetIds.begin(), end_iter = linksetIds.end();
       iter != end_iter; ++iter)
    _cMgr.sendToLinkSetBroadcast(*iter, output_message);
}

libsccp::N_PCSTATE_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_PCSTATE_Indication_message(const common::ITU_PC& affected_point_code,
                                                                             libsccp::N_PCSTATE_IND_Message::signalling_point_status_e signaling_point_status)
{
  libsccp::N_PCSTATE_IND_Message resultMesage;

  resultMesage.setAffectedSignalingPoint(affected_point_code.getValue());
  resultMesage.setSignalingPointStatus(signaling_point_status);

  return resultMesage;
}

libsccp::N_PCSTATE_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_PCSTATE_Indication_message(const common::ITU_PC& affected_point_code,
                                                                             libsccp::N_PCSTATE_IND_Message::signalling_point_status_e signaling_point_status,
                                                                             libsccp::N_PCSTATE_IND_Message::remote_sccp_status_e remote_SCCP_status)
{
  libsccp::N_PCSTATE_IND_Message resultMesage(make_N_PCSTATE_Indication_message(affected_point_code, signaling_point_status));
  resultMesage.setRemoteSCCPStatus(remote_SCCP_status);
  return resultMesage;
}

libsccp::N_COORD_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_COORD_Indication_message(const messages::DRSTMessage& message)
{
  libsccp::N_COORD_IND_Message resultMesage;

  common::ITU_PC affectedPointCode;
  size_t offset=0;
  if ( message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset) ) {
    resultMesage.setSignalingPointCode(affectedPointCode.getValue());

    resultMesage.setSSN(uint8_t(message.getSSN().getValue()));
    if ( message.getSMI().isSetValue() )
      resultMesage.setSubsystemMultiplicityInd(uint8_t(message.getSMI().getValue()));

    return resultMesage;
  } else
    throw smsc::util::Exception("SignalingNetworkManagementMessageHandlers::make_N_STATE_Indication_message::: affected point code is absent in original DRST message");
}

}}}}
