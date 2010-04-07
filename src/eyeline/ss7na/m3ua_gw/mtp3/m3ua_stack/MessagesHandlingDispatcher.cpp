#include "MessagesHandlingDispatcher.hpp"
#include "AspMaintenanceMessageHandlers.hpp"
#include "DATAMessageHandler.hpp"
#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "M3UAManagementMessageHandlers.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

void
ActiveAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
ActiveMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DAVAMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DRSTMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DUNAMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DUPUMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DAUDMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
SCONMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  SignalingNetworkManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DownAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DownMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
ErrorMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  M3UAManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
NotifyMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  M3UAManagementMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
InactiveAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
InactiveMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
UPAckMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
UPMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  AspMaintenanceMessageHandlers msgHndlr;
  msgHndlr.handle(*this, link_id);
}

void
DATAMessage_HandlingDispatcher::dispatch_handle(const common::LinkId& link_id) const
{
  DATAMessageHandler msgHndlr;
  msgHndlr.handle(*this, link_id);
}

}}}}}
