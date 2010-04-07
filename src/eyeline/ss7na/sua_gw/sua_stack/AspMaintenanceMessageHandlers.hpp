#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_ASPMAINTENANCEMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_ASPMAINTENANCEMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/ActiveMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/ActiveAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/InactiveMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/InactiveAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/UPMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/UPAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DownMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DownAckMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class AspMaintenanceMessageHandlers : public utilx::Singleton<AspMaintenanceMessageHandlers> {
public:
  AspMaintenanceMessageHandlers();
  void handle(const messages::ActiveMessage& message, const common::LinkId& link_id);
  void handle(const messages::ActiveAckMessage& message, const common::LinkId& link_id);
  void handle(const messages::InactiveMessage& message, const common::LinkId& link_id);
  void handle(const messages::InactiveAckMessage& message, const common::LinkId& link_id);
  void handle(const messages::UPMessage& message, const common::LinkId& link_id);
  void handle(const messages::UPAckMessage& message, const common::LinkId& link_id);
  void handle(const messages::DownMessage& message, const common::LinkId& link_id);
  void handle(const messages::DownAckMessage& message, const common::LinkId& link_id);

  void setSGPTrafficMode(const std::string& traffic_mode_value);
private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
  uint32_t _trafficMode;
};

}}}}

#endif
