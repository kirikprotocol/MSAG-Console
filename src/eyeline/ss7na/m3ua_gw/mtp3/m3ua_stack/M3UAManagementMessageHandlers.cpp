#include "M3UAManagementMessageHandlers.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
#include "messages/InactiveMessage.hpp"
#include "messages/DownMessage.hpp"
#include "messages/ActiveMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

M3UAManagementMessageHandlers::M3UAManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("m3ua_stack")),
    _cMgr(m3ua_gw::io_dispatcher::ConnectMgr::getInstance()) {}

void
M3UAManagementMessageHandlers::handle(const messages::ErrorMessage& message,
                                     const common::LinkId& link_id)
{
  smsc_log_info(_logger, "M3UAManagementMessageHandlers::handle::: handle ErrorMessage [%s]", message.toString().c_str());
//  try {
//    _cMgr.send(link_id, messages::InactiveMessage());
//  } catch (common::io_dispatcher::ProtocolException& ex) {
//    smsc_log_info(_logger, "M3UAManagementMessageHandlers::handle::: ASP is not in ACTIVE state, send Down mesage [%s]", message.toString().c_str());
//    _cMgr.send(link_id, messages::DownMessage());
//  }
}

void
M3UAManagementMessageHandlers::handle(const messages::NotifyMessage& message,
                                     const common::LinkId& link_id)
{
  smsc_log_info(_logger, "M3UAManagementMessageHandlers::handle::: handle NotifyMessage [%s]", message.toString().c_str());
//  const common::TLV_ApplicationStatus& appStatus = message.getStatus();
//  if ( appStatus.getStatusType() == common::TLV_ApplicationStatus::AS_STATE_CHANGE ) {
//    if ( appStatus.getStatusId() == common::TLV_ApplicationStatus::ASP_ACTIVE_NOTICE ) {
//      RCRegistry::getInstance().insert(link_id, message.getRoutingContext());
//    }
//  }
}

}}}}}
