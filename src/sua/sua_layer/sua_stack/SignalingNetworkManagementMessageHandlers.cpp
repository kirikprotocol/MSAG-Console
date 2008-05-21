#include "SignalingNetworkManagementMessageHandlers.hpp"
#include <sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>

namespace sua_stack {

SignalingNetworkManagementMessageHandlers*
utilx::Singleton<SignalingNetworkManagementMessageHandlers>::_instance;

SignalingNetworkManagementMessageHandlers::SignalingNetworkManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DAVAMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DAVAMessage [%s]", message.toString().c_str());

  if ( message.getSSN().isSetValue() ) {
    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsua_messages::N_STATE_IND_Message::USER_IN_SERVICE));
  } else {
    sua_messages::ITU_PC affectedPointCode;
    while(message.getAffectedPointCode().getNextPC(&affectedPointCode)) {
      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsua_messages::N_PCSTATE_IND_Message::SIGNALING_POINT_ACCESSIBLE));
    }
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DRSTMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DRSTMessage [%s]", message.toString().c_str());
  if ( message.getSSN().isSetValue() &&
       message.getSMI().isSetValue() )
    sendSCCPManagementMessageToApplications(make_N_COORD_Indication_message(message));
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DUNAMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUNAMessage [%s]", message.toString().c_str());

  if ( message.getSSN().isSetValue() ) {
    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsua_messages::N_STATE_IND_Message::USER_OUT_OF_SERVICE));
  } else {
    sua_messages::ITU_PC affectedPointCode;
    while(message.getAffectedPointCode().getNextPC(&affectedPointCode)) {
      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsua_messages::N_PCSTATE_IND_Message::SIGNALING_POINT_INACCESSIBLE));
    }
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DUPUMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUPUMessage [%s]", message.toString().c_str());
  sua_messages::ITU_PC affectedPointCode;
  while(message.getAffectedPointCode().getNextPC(&affectedPointCode)) {
    sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsua_messages::N_PCSTATE_IND_Message::SIGNALING_POINT_INACCESSIBLE, uint8_t(message.getUserCause().getCause())));
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::SCONMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle SCONMessage [%s]", message.toString().c_str());

  if ( message.getSSN().isSetValue() ) {
    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsua_messages::N_STATE_IND_Message::USER_OUT_OF_SERVICE));
  } else {
    sua_messages::ITU_PC affectedPointCode;
    while(message.getAffectedPointCode().getNextPC(&affectedPointCode)) {
      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsua_messages::N_PCSTATE_IND_Message::SIGNALING_POINT_CONGESTED));
    }
  }
}

void
SignalingNetworkManagementMessageHandlers::sendSCCPManagementMessageToApplications(const libsua_messages::LibsuaMessage& outputMessage)
{
  std::set<communication::LinkId> linksetIds = sua_user_communication::LinkSetInfoRegistry::getInstance().getLinkSetIds();
  for (std::set<communication::LinkId>::const_iterator iter = linksetIds.begin(), end_iter = linksetIds.end();
       iter != end_iter; ++iter)
    _cMgr.sendToLinkSetBroadcast(*iter, outputMessage);
}

libsua_messages::N_PCSTATE_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_PCSTATE_Indication_message(const sua_messages::ITU_PC& affectedPointCode, uint8_t signalingPointStatus)
{
  libsua_messages::N_PCSTATE_IND_Message resultMesage;

  uint16_t pointCode;
  memcpy(reinterpret_cast<uint8_t*>(&pointCode), affectedPointCode.getValue()+2, sizeof(pointCode));
  pointCode = ntohs(pointCode);
  
  resultMesage.setAffectedSignalingPoint(pointCode);
  resultMesage.setSignalingPointStatus(signalingPointStatus);

  return resultMesage;
}

libsua_messages::N_PCSTATE_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_PCSTATE_Indication_message(const sua_messages::ITU_PC& affectedPointCode, uint8_t signalingPointStatus, uint8_t remoteSCCPStatus)
{
  libsua_messages::N_PCSTATE_IND_Message resultMesage(make_N_PCSTATE_Indication_message(affectedPointCode, signalingPointStatus));
  resultMesage.setRemoteSCCPStatus(remoteSCCPStatus);
  return resultMesage;
}

libsua_messages::N_COORD_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_COORD_Indication_message(const sua_messages::DRSTMessage& message)
{
  libsua_messages::N_COORD_IND_Message resultMesage;

  sua_messages::ITU_PC affectedPointCode;
  if ( message.getAffectedPointCode().getNextPC(&affectedPointCode) ) {
    uint16_t pointCode;
    memcpy(reinterpret_cast<uint8_t*>(&pointCode), affectedPointCode.getValue()+2, sizeof(pointCode));
    pointCode = ntohs(pointCode);
    resultMesage.setSignalingPointCode(pointCode);

    resultMesage.setSSN(uint8_t(message.getSSN().getValue()));
    if ( message.getSMI().isSetValue() )
      resultMesage.setSubsystemMultiplicityInd(uint8_t(message.getSMI().getValue()));

    return resultMesage;
  } else
    throw smsc::util::Exception("SignalingNetworkManagementMessageHandlers::make_N_STATE_Indication_message::: affected point code is absent in original DRST message");
}

}
