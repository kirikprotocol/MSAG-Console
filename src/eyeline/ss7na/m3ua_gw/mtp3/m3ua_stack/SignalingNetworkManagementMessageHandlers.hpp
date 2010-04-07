#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"

# include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DAVAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DRSTMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DUNAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DUPUMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DAUDMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/SCONMessage.hpp"

# include "eyeline/ss7na/libsccp/messages/N_STATE_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_PCSTATE_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_COORD_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

class SignalingNetworkManagementMessageHandlers {
public:
  SignalingNetworkManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("m3ua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _mtp3Instance(sap::MTPSapInstance::getInstance())
  {}

  void handle(const messages::DAVAMessage& message, const common::LinkId& link_id);
  void handle(const messages::DRSTMessage& message, const common::LinkId& link_id);
  void handle(const messages::DUNAMessage& message, const common::LinkId& link_id);
  void handle(const messages::DUPUMessage& message, const common::LinkId& link_id);
  void handle(const messages::DAUDMessage& message, const common::LinkId& link_id);
  void handle(const messages::SCONMessage& message, const common::LinkId& link_id);
private:
  template <class MSG>
  libsccp::N_STATE_IND_Message
  make_N_STATE_Indication_message(const MSG& message, uint8_t user_status);

  libsccp::N_PCSTATE_IND_Message
  make_N_PCSTATE_Indication_message(const common::ITU_PC& affected_point_code, uint8_t signaling_point_status);

  libsccp::N_PCSTATE_IND_Message
  make_N_PCSTATE_Indication_message(const common::ITU_PC& affected_point_code, uint8_t signaling_point_status, uint8_t remote_SCCP_status);

  libsccp::N_COORD_IND_Message
  make_N_COORD_Indication_message(const messages::DRSTMessage& message);

  void sendSCCPManagementMessageToApplications(const libsccp::LibsccpMessage& output_message);
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
  sap::MTPSapInstance& _mtp3Instance;
};

}}}}}

#endif

