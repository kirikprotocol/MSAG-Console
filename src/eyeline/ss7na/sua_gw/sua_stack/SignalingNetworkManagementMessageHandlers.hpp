#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"

# include "eyeline/ss7na/sua_gw/sua_stack/messages/DAVAMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DRSTMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DUNAMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DUPUMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SCONMessage.hpp"

# include "eyeline/ss7na/libsccp/messages/N_STATE_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_PCSTATE_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_COORD_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class SignalingNetworkManagementMessageHandlers
  : public utilx::Singleton<SignalingNetworkManagementMessageHandlers> {
public:
  SignalingNetworkManagementMessageHandlers();
  void handle(const messages::DAVAMessage& message, const common::LinkId& link_id);
  void handle(const messages::DRSTMessage& message, const common::LinkId& link_id);
  void handle(const messages::DUNAMessage& message, const common::LinkId& link_id);
  void handle(const messages::DUPUMessage& message, const common::LinkId& link_id);
  void handle(const messages::SCONMessage& message, const common::LinkId& link_id);
private:
  template <class MSG>
  libsccp::N_STATE_IND_Message
  make_N_STATE_Indication_message(const MSG& message,
                                  uint8_t user_status);

  libsccp::N_PCSTATE_IND_Message
  make_N_PCSTATE_Indication_message(const common::ITU_PC& affected_point_code,
                                    libsccp::N_PCSTATE_IND_Message::signalling_point_status_e signaling_point_status);

  libsccp::N_PCSTATE_IND_Message
  make_N_PCSTATE_Indication_message(const common::ITU_PC& affected_point_code,
                                    libsccp::N_PCSTATE_IND_Message::signalling_point_status_e signaling_point_status,
                                    libsccp::N_PCSTATE_IND_Message::remote_sccp_status_e remote_SCCP_status);

  libsccp::N_COORD_IND_Message
  make_N_COORD_Indication_message(const messages::DRSTMessage& message);

  void sendSCCPManagementMessageToApplications(const libsccp::LibsccpMessage& output_message);
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

# include "eyeline/ss7na/sua_gw/sua_stack/SignalingNetworkManagementMessageHandlers_impl.hpp"

}}}}

#endif

