#include "SUAManagementMessageHandlers.hpp"
#include "RCRegistry.hpp"
#include <eyeline/sua/communication/sua_messages/InactiveMessage.hpp>
#include <eyeline/sua/communication/sua_messages/DownMessage.hpp>
#include <eyeline/sua/communication/sua_messages/ActiveMessage.hpp>
#include <eyeline/sua/sua_layer/io_dispatcher/Exceptions.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

SUAManagementMessageHandlers::SUAManagementMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
SUAManagementMessageHandlers::handle(const communication::sua_messages::ErrorMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: handle ErrorMessage [%s]", message.toString().c_str());
  try {
    _cMgr.send(linkId, communication::sua_messages::InactiveMessage());
  } catch (io_dispatcher::ProtocolException& ex) {
    smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: ASP is not in ACTIVE state, send Down mesage [%s]", message.toString().c_str());
    _cMgr.send(linkId, communication::sua_messages::DownMessage());
  }
}

void
SUAManagementMessageHandlers::handle(const communication::sua_messages::NotifyMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "SUAManagementMessageHandlers::handle::: handle NotifyMessage [%s]", message.toString().c_str());
  const communication::sua_messages::TLV_ApplicationStatus& appStatus = message.getStatus();
  if ( appStatus.getStatusType() == communication::sua_messages::TLV_ApplicationStatus::AS_STATE_CHANGE ) {
    if ( appStatus.getStatusId() == communication::sua_messages::TLV_ApplicationStatus::ASP_ACTIVE_NOTICE ) {
      RCRegistry::getInstance().insert(linkId, message.getRoutingContext());
    }
  }
}

}}}}
