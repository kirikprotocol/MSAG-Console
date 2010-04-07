#include "MessagesHandlingDispatcher.hpp"
#include "AspMaintenanceMessageHandlers.hpp"
#include "CLCOMessageHandlers.hpp"
#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "SUAManagementMessageHandlers.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

void
ActiveAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
ActiveMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
CLDTMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  CLCOMessageHandlers::getInstance().handle(*this, link_id);
}

void
CLDRMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  CLCOMessageHandlers::getInstance().handle(*this, link_id);
}

void
DAVAMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
DRSTMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
DUNAMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
DUPUMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
SCONMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
DownAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
DownMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
ErrorMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SUAManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
NotifyMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SUAManagementMessageHandlers::getInstance().handle(*this, link_id);
}

void
InactiveAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
InactiveMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
UPAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

void
UPMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers::getInstance().handle(*this, link_id);
}

}}}}
