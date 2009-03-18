#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__

# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/communication/LinkId.hpp>

# include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

# include <eyeline/sua/communication/sua_messages/DAVAMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DRSTMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DUNAMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DUPUMessage.hpp>
# include <eyeline/sua/communication/sua_messages/SCONMessage.hpp>

# include <eyeline/sua/communication/libsua_messages/N_STATE_IND_Message.hpp>
# include <eyeline/sua/communication/libsua_messages/N_PCSTATE_IND_Message.hpp>
# include <eyeline/sua/communication/libsua_messages/N_COORD_IND_Message.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class SignalingNetworkManagementMessageHandlers
  : public utilx::Singleton<SignalingNetworkManagementMessageHandlers> {
public:
  SignalingNetworkManagementMessageHandlers();
  void handle(const communication::sua_messages::DAVAMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::DRSTMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::DUNAMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::DUPUMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::SCONMessage& message, const communication::LinkId& linkId);
private:
  template <class MSG>
  communication::libsua_messages::N_STATE_IND_Message make_N_STATE_Indication_message(const MSG& message, uint8_t userStatus);

  communication::libsua_messages::N_PCSTATE_IND_Message make_N_PCSTATE_Indication_message(const communication::sua_messages::ITU_PC& affectedPointCode, uint8_t signalingPointStatus);
  communication::libsua_messages::N_PCSTATE_IND_Message make_N_PCSTATE_Indication_message(const communication::sua_messages::ITU_PC& affectedPointCode, uint8_t signalingPointStatus, uint8_t remoteSCCPStatus);
  communication::libsua_messages::N_COORD_IND_Message make_N_COORD_Indication_message(const communication::sua_messages::DRSTMessage& message);
  void sendSCCPManagementMessageToApplications(const communication::libsua_messages::LibsuaMessage& outputMessage);
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

# include <eyeline/sua/sua_layer/sua_stack/SignalingNetworkManagementMessageHandlers_impl.hpp>

}}}}

#endif

