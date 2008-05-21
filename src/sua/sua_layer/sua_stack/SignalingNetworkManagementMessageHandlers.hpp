#ifndef __SUA_STACK_COMMUNICATION_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__
# define __SUA_STACK_COMMUNICATION_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__ 1

# include <sua/utilx/Singleton.hpp>
# include <logger/Logger.h>
# include <sua/communication/LinkId.hpp>

# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

# include <sua/communication/sua_messages/DAVAMessage.hpp>
# include <sua/communication/sua_messages/DRSTMessage.hpp>
# include <sua/communication/sua_messages/DUNAMessage.hpp>
# include <sua/communication/sua_messages/DUPUMessage.hpp>
# include <sua/communication/sua_messages/SCONMessage.hpp>

# include <sua/communication/libsua_messages/N_STATE_IND_Message.hpp>
# include <sua/communication/libsua_messages/N_PCSTATE_IND_Message.hpp>
# include <sua/communication/libsua_messages/N_COORD_IND_Message.hpp>

namespace sua_stack {

class SignalingNetworkManagementMessageHandlers
  : public utilx::Singleton<SignalingNetworkManagementMessageHandlers> {
public:
  SignalingNetworkManagementMessageHandlers();
  void handle(const sua_messages::DAVAMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DRSTMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DUNAMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DUPUMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::SCONMessage& message, const communication::LinkId& linkId);
private:
  template <class MSG>
  libsua_messages::N_STATE_IND_Message make_N_STATE_Indication_message(const MSG& message, uint8_t userStatus);

  libsua_messages::N_PCSTATE_IND_Message make_N_PCSTATE_Indication_message(const sua_messages::ITU_PC& affectedPointCode, uint8_t signalingPointStatus);
  libsua_messages::N_PCSTATE_IND_Message make_N_PCSTATE_Indication_message(const sua_messages::ITU_PC& affectedPointCode, uint8_t signalingPointStatus, uint8_t remoteSCCPStatus);
  libsua_messages::N_COORD_IND_Message make_N_COORD_Indication_message(const sua_messages::DRSTMessage& message);
  void sendSCCPManagementMessageToApplications(const libsua_messages::LibsuaMessage& outputMessage);
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

# include <sua/sua_layer/sua_stack/SignalingNetworkManagementMessageHandlers_impl.hpp>

}

#endif

