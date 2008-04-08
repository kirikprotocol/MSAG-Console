#include "SUAManagementMessageHandlers.hpp"

namespace sua_stack {

SUAManagementMessageHandlers*
utilx::Singleton<SUAManagementMessageHandlers>::_instance;

SUAManagementMessageHandlers::SUAManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SUAManagementMessageHandlers::handle(const sua_messages::ErrorMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SUAManagementMessageHandlers::handle::: handle ErrorMessage [%s]", message.toString().c_str());
}

void
SUAManagementMessageHandlers::handle(const sua_messages::NotifyMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SUAManagementMessageHandlers::handle::: handle NotifyMessage [%s]", message.toString().c_str());
}

}
