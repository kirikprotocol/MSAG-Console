#include <sua/communication/sua_messages/ActiveAckMessage.hpp>
#include <sua/communication/sua_messages/CLDTMessage.hpp>
#include <sua/communication/sua_messages/DAUDMessage.hpp>
#include <sua/communication/sua_messages/DAVAMessage.hpp>
#include <sua/communication/sua_messages/DRSTMessage.hpp>
#include <sua/communication/sua_messages/DUNAMessage.hpp>
#include <sua/communication/sua_messages/DUPUMessage.hpp>
#include <sua/communication/sua_messages/DownAckMessage.hpp>
#include <sua/communication/sua_messages/DownMessage.hpp>
#include <sua/communication/sua_messages/ErrorMessage.hpp>
#include <sua/communication/sua_messages/InactiveAckMessage.hpp>
#include <sua/communication/sua_messages/InactiveMessage.hpp>
#include <sua/communication/sua_messages/NotifyMessage.hpp>
#include <sua/communication/sua_messages/SCONMessage.hpp>
#include <sua/communication/sua_messages/UPAckMessage.hpp>
#include <sua/communication/sua_messages/UPMessage.hpp>

#include "AspMaintenanceMessageHandlers.hpp"
#include "CLCOMessageHandlers.hpp"
#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "SUAManagementMessageHandlers.hpp"

namespace sua_messages {

void
ActiveAckMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
ActiveMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
CLDTMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::CLCOMessageHandlers::getInstance().handle(*this, linkId);
}

void
DAUDMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DAVAMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DRSTMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DUNAMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DUPUMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
SCONMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SignalingNetworkManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
DownAckMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
DownMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
ErrorMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SUAManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
NotifyMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::SUAManagementMessageHandlers::getInstance().handle(*this, linkId);
}

void
InactiveAckMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
InactiveMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
UPAckMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

void
UPMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_stack::AspMaintenanceMessageHandlers::getInstance().handle(*this, linkId);
}

}
