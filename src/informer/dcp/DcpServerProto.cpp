#include "DcpServerProto.hpp"
#include "messages/UserAuth.hpp"
#include "messages/GetUserStats.hpp"
#include "messages/CreateDelivery.hpp"
#include "messages/ModifyDelivery.hpp"
#include "messages/DropDelivery.hpp"
#include "messages/ChangeDeliveryState.hpp"
#include "messages/AddDeliveryMessages.hpp"
#include "messages/DropDeliveryMessages.hpp"
#include "messages/GetDeliveryGlossary.hpp"
#include "messages/ModifyDeliveryGlossary.hpp"
#include "messages/GetDeliveryState.hpp"
#include "messages/GetDeliveryInfo.hpp"
#include "messages/GetDeliveriesList.hpp"
#include "messages/GetDeliveriesListNext.hpp"
#include "messages/CountDeliveries.hpp"
#include "messages/CountDeliveriesNext.hpp"
#include "messages/GetDeliveryHistory.hpp"
#include "messages/RequestMessagesState.hpp"
#include "messages/GetNextMessagesPack.hpp"
#include "messages/CountMessages.hpp"
#include "messages/CountMessagesPack.hpp"

namespace eyeline {
namespace informer {
namespace dcp {

void DcpServerProto::assignHandler(DcpHandler* newHandler)
{
  handler=newHandler;
}

void DcpServerProto::decodeAndHandleMessage(const char* buf,size_t sz,int connId)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss,connId);
}

void DcpServerProto::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss,int connId)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_UserAuth:
    {
      messages::UserAuth msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetUserStats:
    {
      messages::GetUserStats msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CreateDelivery:
    {
      messages::CreateDelivery msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_ModifyDelivery:
    {
      messages::ModifyDelivery msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_DropDelivery:
    {
      messages::DropDelivery msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_ChangeDeliveryState:
    {
      messages::ChangeDeliveryState msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_AddDeliveryMessages:
    {
      messages::AddDeliveryMessages msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_DropDeliveryMessages:
    {
      messages::DropDeliveryMessages msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryGlossary:
    {
      messages::GetDeliveryGlossary msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_ModifyDeliveryGlossary:
    {
      messages::ModifyDeliveryGlossary msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryState:
    {
      messages::GetDeliveryState msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryInfo:
    {
      messages::GetDeliveryInfo msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveriesList:
    {
      messages::GetDeliveriesList msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveriesListNext:
    {
      messages::GetDeliveriesListNext msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CountDeliveries:
    {
      messages::CountDeliveries msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CountDeliveriesNext:
    {
      messages::CountDeliveriesNext msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryHistory:
    {
      messages::GetDeliveryHistory msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_RequestMessagesState:
    {
      messages::RequestMessagesState msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetNextMessagesPack:
    {
      messages::GetNextMessagesPack msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CountMessages:
    {
      messages::CountMessages msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CountMessagesPack:
    {
      messages::CountMessagesPack msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    default:
      throw eyeline::protogen::framework::UnhandledMessage(tag);
  }
}


}
}
}
