#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Pause_Ind.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Resume_Ind.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Status_Ind.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
#include "eyeline/ss7na/m3ua_gw/point_status_mgmt/PointStatusTable.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DAVAMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DAVAMessage [%s]", message.toString().c_str());

  common::ITU_PC affectedPointCode;
  size_t offset=0;
  while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
    mtp3::primitives::MTP_Resume_Ind mtpPrimitive(affectedPointCode.getValue());
    point_status_mgmt::PointStatusTable::getInstance().updateStatus(mtpPrimitive.getAffectedDPC(),
                                                                    point_status_mgmt::POINT_ALLOWED);
    _mtp3Instance.handle(mtpPrimitive);
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DRSTMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DRSTMessage [%s]", message.toString().c_str());
//  if ( message.getSSN().isSetValue() &&
//       message.getSMI().isSetValue() )
//    sendSCCPManagementMessageToApplications(make_N_COORD_Indication_message(message));
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DUNAMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUNAMessage [%s]", message.toString().c_str());
  common::ITU_PC affectedPointCode;
  size_t offset=0;
  while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
    mtp3::primitives::MTP_Pause_Ind mtpPrimitive(affectedPointCode.getValue());
    point_status_mgmt::PointStatusTable::getInstance().updateStatus(mtpPrimitive.getAffectedDPC(),
                                                                    point_status_mgmt::POINT_PROHIBITED);
    _mtp3Instance.handle(mtpPrimitive);
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DUPUMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUPUMessage [%s]", message.toString().c_str());
  common::ITU_PC affectedPointCode;
  size_t offset=0;
  while(message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset)) {
    mtp3::primitives::MTP_Status_Ind mtpPrimitive(affectedPointCode.getValue(), message.getUserCause().getCause());
    point_status_mgmt::PointStatusTable::getInstance().updateStatus(mtpPrimitive.getAffectedDPC(),
                                                                    point_status_mgmt::POINT_PROHIBITED);
    _mtp3Instance.handle(mtpPrimitive);
  }
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::DAUDMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DAUDMessage [%s]", message.toString().c_str());
}

void
SignalingNetworkManagementMessageHandlers::handle(const messages::SCONMessage& message,
                                                  const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle SCONMessage [%s]", message.toString().c_str());
//
//  if ( message.getSSN().isSetValue() ) {
//    sendSCCPManagementMessageToApplications(make_N_STATE_Indication_message(message, libsccp::N_STATE_IND_Message::USER_OUT_OF_SERVICE));
//  } else {
//    common::ITU_PC affectedPointCode;
//    while(message.getAffectedPointCode().getNextPC(&affectedPointCode)) {
//      sendSCCPManagementMessageToApplications(make_N_PCSTATE_Indication_message(affectedPointCode, libsccp::N_PCSTATE_IND_Message::SIGNALING_POINT_CONGESTED));
//    }
//  }
}

}}}}}
