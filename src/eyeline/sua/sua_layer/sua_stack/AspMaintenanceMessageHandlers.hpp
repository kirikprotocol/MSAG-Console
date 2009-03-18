#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_ASPMAINTENANCEMESSAGEHANDLERS_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_ASPMAINTENANCEMESSAGEHANDLERS_HPP__

# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>

# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
# include <eyeline/sua/communication/sua_messages/ActiveMessage.hpp>
# include <eyeline/sua/communication/sua_messages/ActiveAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/InactiveMessage.hpp>
# include <eyeline/sua/communication/sua_messages/InactiveAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/UPMessage.hpp>
# include <eyeline/sua/communication/sua_messages/UPAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DownMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DownAckMessage.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class AspMaintenanceMessageHandlers : public utilx::Singleton<AspMaintenanceMessageHandlers> {
public:
  AspMaintenanceMessageHandlers();
  void handle(const communication::sua_messages::ActiveMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::ActiveAckMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::InactiveMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::InactiveAckMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::UPMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::UPAckMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::DownMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::DownAckMessage& message, const communication::LinkId& linkId);

  void setSGPTrafficMode(const std::string& trafficModeValue);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
  uint32_t _trafficMode;
};

}}}}

#endif
