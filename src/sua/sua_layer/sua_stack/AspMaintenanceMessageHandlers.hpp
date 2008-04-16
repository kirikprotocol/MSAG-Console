#ifndef __SUA_STACK_COMMUNICATION_ASPMAINTENANCEMESSAGEHANDLERS_HPP__
# define __SUA_STACK_COMMUNICATION_ASPMAINTENANCEMESSAGEHANDLERS_HPP__ 1

# include <sua/utilx/Singleton.hpp>
# include <logger/Logger.h>
# include <sua/communication/LinkId.hpp>

# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

# include <sua/communication/sua_messages/ActiveMessage.hpp>
# include <sua/communication/sua_messages/ActiveAckMessage.hpp>
# include <sua/communication/sua_messages/InactiveMessage.hpp>
# include <sua/communication/sua_messages/InactiveAckMessage.hpp>
# include <sua/communication/sua_messages/UPMessage.hpp>
# include <sua/communication/sua_messages/UPAckMessage.hpp>
# include <sua/communication/sua_messages/DownMessage.hpp>
# include <sua/communication/sua_messages/DownAckMessage.hpp>

namespace sua_stack {

class AspMaintenanceMessageHandlers : public utilx::Singleton<AspMaintenanceMessageHandlers> {
public:
  AspMaintenanceMessageHandlers();
  void handle(const sua_messages::ActiveMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::ActiveAckMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::InactiveMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::InactiveAckMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::UPMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::UPAckMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DownMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::DownAckMessage& message, const communication::LinkId& linkId);

  void setSGPTrafficMode(const std::string& trafficModeValue);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
  uint32_t _trafficMode;
};

}

#endif
