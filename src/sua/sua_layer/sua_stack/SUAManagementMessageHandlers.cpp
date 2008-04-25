#include "SUAManagementMessageHandlers.hpp"
#include "RCRegistry.hpp"
#include <sua/communication/sua_messages/InactiveMessage.hpp>
#include <sua/communication/sua_messages/DownMessage.hpp>
#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>
#include <sua/communication/sua_messages/ActiveMessage.hpp>

namespace sua_stack {

SUAManagementMessageHandlers*
utilx::Singleton<SUAManagementMessageHandlers>::_instance;

SUAManagementMessageHandlers::SUAManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SUAManagementMessageHandlers::handle(const sua_messages::ErrorMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: handle ErrorMessage [%s]", message.toString().c_str());
  try {
    _cMgr.send(linkId, sua_messages::InactiveMessage());
  } catch (io_dispatcher::ProtocolException& ex) {
    smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: ASP is not in ACTIVE state, send Down mesage [%s]", message.toString().c_str());
    _cMgr.send(linkId, sua_messages::DownMessage());
  }
}

void
SUAManagementMessageHandlers::handle(const sua_messages::NotifyMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: handle NotifyMessage [%s]", message.toString().c_str());
  const sua_messages::TLV_ApplicationStatus& appStatus = message.getStatus();
  if ( appStatus.getStatusType() == sua_messages::TLV_ApplicationStatus::AS_STATE_CHANGE ) {
    if ( appStatus.getStatusId() == sua_messages::TLV_ApplicationStatus::ASP_ACTIVE_NOTICE ) {
      RCRegistry::getInstance().insert(linkId, message.getRoutingContext());
    }
  }
}

}
