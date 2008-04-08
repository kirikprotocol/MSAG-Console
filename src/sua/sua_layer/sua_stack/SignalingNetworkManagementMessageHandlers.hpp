#ifndef __SUA_STACK_COMMUNICATION_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__
# define __SUA_STACK_COMMUNICATION_SIGNALINGNETWORKMANAGEMENTMESSAGEHANDLERS_HPP__ 1

# include <sua/utilx/Singleton.hpp>
# include <logger/Logger.h>
# include <sua/communication/LinkId.hpp>

# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

# include <sua/communication/sua_messages/DAUDMessage.hpp>
# include <sua/communication/sua_messages/DAVAMessage.hpp>
# include <sua/communication/sua_messages/DRSTMessage.hpp>
# include <sua/communication/sua_messages/DUNAMessage.hpp>
# include <sua/communication/sua_messages/DUPUMessage.hpp>
# include <sua/communication/sua_messages/SCONMessage.hpp>

namespace sua_stack {

class SignalingNetworkManagementMessageHandlers
  : public utilx::Singleton<SignalingNetworkManagementMessageHandlers> {
public:
  SignalingNetworkManagementMessageHandlers();
  void handle(const sua_messages::DAUDMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DAVAMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DRSTMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DUNAMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DUPUMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::SCONMessage& message, const communication::LinkId& linkId);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

}

#endif

