#include "SignalingNetworkManagementMessageHandlers.hpp"

namespace sua_stack {

SignalingNetworkManagementMessageHandlers*
utilx::Singleton<SignalingNetworkManagementMessageHandlers>::_instance;

SignalingNetworkManagementMessageHandlers::SignalingNetworkManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DAUDMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DAUDMessage [%s]", message.toString().c_str());
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DAVAMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DAVAMessage [%s]", message.toString().c_str());
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DRSTMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DRSTMessage [%s]", message.toString().c_str());
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DUNAMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUNAMessage [%s]", message.toString().c_str());
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::DUPUMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle DUPUMessage [%s]", message.toString().c_str());
}

void
SignalingNetworkManagementMessageHandlers::handle(const sua_messages::SCONMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "SignalingNetworkManagementMessageHandlers::handle::: handle SCONMessage [%s]", message.toString().c_str());
}

}
