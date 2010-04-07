#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_ASPMAINTENANCEMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_ASPMAINTENANCEMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/ActiveMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/ActiveAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/InactiveMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/InactiveAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/UPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/UPAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DownMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DownAckMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

class AspMaintenanceMessageHandlers {
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

  static void setSGPTrafficMode(const std::string& traffic_mode_value);
private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
  static uint32_t _trafficMode;
};

}}}}}

#endif
