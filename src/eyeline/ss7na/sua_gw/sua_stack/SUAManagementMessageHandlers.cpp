#include "SUAManagementMessageHandlers.hpp"
#include "RCRegistry.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "messages/InactiveMessage.hpp"
#include "messages/DownMessage.hpp"
#include "messages/ActiveMessage.hpp"
#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

SUAManagementMessageHandlers::SUAManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SUAManagementMessageHandlers::handle(const messages::ErrorMessage& message,
                                     const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: handle ErrorMessage [%s]", message.toString().c_str());
  try {
    try {
      _cMgr.send(link_id, messages::InactiveMessage());
    } catch (common::io_dispatcher::ProtocolException& ex) {
      smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: ASP is not in ACTIVE state, send Down mesage [%s]", message.toString().c_str());
      _cMgr.send(link_id, messages::DownMessage());
    }
  } catch(corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "CLCOMessageHandlers::handle::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), link_id.getValue().c_str());
    common::io_dispatcher::LinkPtr brokeConn = _cMgr.removeLink(link_id);
    if ( brokeConn.Get() )
      common::io_dispatcher::Reconnector::getInstance().schedule(brokeConn);
  }
}

void
SUAManagementMessageHandlers::handle(const messages::NotifyMessage& message,
                                     const common::LinkId& link_id)
{
  smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: handle NotifyMessage [%s]", message.toString().c_str());
  const common::TLV_ApplicationStatus& appStatus = message.getStatus();
  if ( appStatus.getStatusType() == common::TLV_ApplicationStatus::AS_STATE_CHANGE ) {
    if ( appStatus.getStatusId() == common::TLV_ApplicationStatus::ASP_ACTIVE_NOTICE ) {
      RCRegistry::getInstance().insert(link_id, message.getRoutingContext());
    }
  }
}

}}}}
