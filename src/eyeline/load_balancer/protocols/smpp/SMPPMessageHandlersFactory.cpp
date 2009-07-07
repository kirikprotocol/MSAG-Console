#include "SMPPMessageHandlersFactory.hpp"
#include "AlertNotification.hpp"
#include "BindReceiver.hpp"
#include "BindReceiverResp.hpp"
#include "BindTransceiver.hpp"
#include "BindTransceiverResp.hpp"
#include "BindTransmitter.hpp"
#include "BindTransmitterResp.hpp"
#include "CancelSm.hpp"
#include "CancelSmResp.hpp"
#include "DataSm.hpp"
#include "DataSmResp.hpp"
#include "DeliverSm.hpp"
#include "DeliverSmResp.hpp"
#include "EnquireLink.hpp"
#include "EnquireLinkResp.hpp"
#include "GenericNack.hpp"
#include "QuerySm.hpp"
#include "QuerySmResp.hpp"
#include "ReplaceSm.hpp"
#include "ReplaceSmResp.hpp"
#include "SubmitMulti.hpp"
#include "SubmitMultiResp.hpp"
#include "SubmitSm.hpp"
#include "SubmitSmResp.hpp"
#include "Unbind.hpp"
#include "UnbindResp.hpp"

#include "ProtocolManagementMessageHandlers.hpp"
#include "SessionManagementMessageHandlers.hpp"
#include "ServiceMessageHandlers.hpp"
#include "ApplicationMessageHandlers.hpp"

#include "SMPPMessageHandler.hpp"

#include "util/Exception.hpp"
#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

io_subsystem::MessageHandler*
SMPPMessageHandlersFactory::createMessageHandler(const io_subsystem::Packet* packet)
{
  smsc_log_debug(_logger, "SMPPMessageHandlersFactory::createMessageHandler::: get message handler for smpp message with code=0x%08X",
                 packet->packet_type);
  switch(packet->packet_type) {
  case AlertNotification::_MSGCODE:
    return
      instantiateMessageHandler<AlertNotification, ProtocolManagementMessageHandlers>(packet);
  case BindReceiver::_MSGCODE:
    return
      instantiateMessageHandler<BindReceiver, SessionManagementMessageHandlers>(packet);
  case BindReceiverResp::_MSGCODE:
    return
      instantiateMessageHandler<BindReceiverResp, SessionManagementMessageHandlers>(packet);
  case BindTransceiver::_MSGCODE:
    return
      instantiateMessageHandler<BindTransceiver, SessionManagementMessageHandlers>(packet);
  case BindTransceiverResp::_MSGCODE:
    return
      instantiateMessageHandler<BindTransceiverResp, SessionManagementMessageHandlers>(packet);
  case BindTransmitter::_MSGCODE:
    return
      instantiateMessageHandler<BindTransmitter, SessionManagementMessageHandlers>(packet);
  case BindTransmitterResp::_MSGCODE:
    return
      instantiateMessageHandler<BindTransmitterResp, SessionManagementMessageHandlers>(packet);
  case CancelSm::_MSGCODE:
    return
      instantiateMessageHandler<CancelSm, ServiceMessageHandlers>(packet);
  case CancelSmResp::_MSGCODE:
    return
      instantiateMessageHandler<CancelSmResp, ServiceMessageHandlers>(packet);
  case DataSm::_MSGCODE:
    return
      instantiateMessageHandler<DataSm, ApplicationMessageHandlers>(packet);
  case DataSmResp::_MSGCODE:
    return
      instantiateMessageHandler<DataSmResp, ApplicationMessageHandlers>(packet);
  case DeliverSm::_MSGCODE:
    return
      instantiateMessageHandler<DeliverSm, ApplicationMessageHandlers>(packet);
  case DeliverSmResp::_MSGCODE:
    return
      instantiateMessageHandler<DeliverSmResp, ApplicationMessageHandlers>(packet);
  case EnquireLink::_MSGCODE:
    return
      instantiateMessageHandler<EnquireLink, SessionManagementMessageHandlers>(packet);
  case EnquireLinkResp::_MSGCODE:
    return
      instantiateMessageHandler<EnquireLinkResp, SessionManagementMessageHandlers>(packet);
  case GenericNack::_MSGCODE:
    return
      instantiateMessageHandler<GenericNack, ProtocolManagementMessageHandlers>(packet);
  case QuerySm::_MSGCODE:
    return
      instantiateMessageHandler<QuerySm, ServiceMessageHandlers>(packet);
  case QuerySmResp::_MSGCODE:
    return
      instantiateMessageHandler<QuerySmResp, ServiceMessageHandlers>(packet);
  case ReplaceSm::_MSGCODE:
    return
      instantiateMessageHandler<ReplaceSm, ServiceMessageHandlers>(packet);
  case ReplaceSmResp::_MSGCODE:
    return
      instantiateMessageHandler<ReplaceSmResp, ServiceMessageHandlers>(packet);
  case SubmitMulti::_MSGCODE:
    return
      instantiateMessageHandler<SubmitMulti, ApplicationMessageHandlers>(packet);
  case SubmitMultiResp::_MSGCODE:
    return
      instantiateMessageHandler<SubmitMultiResp, ApplicationMessageHandlers>(packet);
  case SubmitSm::_MSGCODE:
    return
      instantiateMessageHandler<SubmitSm, ApplicationMessageHandlers>(packet);
  case SubmitSmResp::_MSGCODE:
    return
      instantiateMessageHandler<SubmitSmResp, ApplicationMessageHandlers>(packet);
  case Unbind::_MSGCODE:
    return
      instantiateMessageHandler<Unbind, SessionManagementMessageHandlers>(packet);
  case UnbindResp::_MSGCODE:
    return
      instantiateMessageHandler<UnbindResp, SessionManagementMessageHandlers>(packet);
  default:
    throw smsc::util::Exception("SMPPMessageHandlersFactory::createMessageHandler::: invalid command id=0x%08X in packet", packet->packet_type);
  }
}

}}}}
