#include "MessagesHandlingDispatcher.hpp"
#include "AspMaintenanceMessageHandlers.hpp"
#include "CLCOMessageHandlers.hpp"
#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "SUAManagementMessageHandlers.hpp"

namespace sua_stack {

void
ActiveAckMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
ActiveMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
CLDTMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::CLCOMessageHandlers::getInstance().handle(*this, linkId);
}

void
CLDRMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::CLCOMessageHandlers::getInstance().handle(*this, linkId);
}

void
DAVAMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DRSTMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DUNAMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DUPUMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
SCONMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DownAckMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
DownMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
ErrorMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SUAManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
NotifyMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SUAManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
InactiveAckMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
InactiveMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
UPAckMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
UPMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

}
