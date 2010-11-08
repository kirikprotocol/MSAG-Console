#include "DcpClient.hpp"
#include "../messages/OkResponse.hpp"
#include "../messages/FailResponse.hpp"
#include "../messages/GetUserStatsResp.hpp"
#include "../messages/CreateDeliveryResp.hpp"
#include "../messages/AddDeliveryMessagesResp.hpp"
#include "../messages/GetDeliveryGlossaryResp.hpp"
#include "../messages/GetDeliveryStateResp.hpp"
#include "../messages/GetDeliveryInfoResp.hpp"
#include "../messages/GetDeliveriesListResp.hpp"
#include "../messages/GetDeliveriesListNextResp.hpp"
#include "../messages/CountDeliveriesResp.hpp"
#include "../messages/GetDeliveryHistoryResp.hpp"
#include "../messages/RequestMessagesStateResp.hpp"
#include "../messages/GetNextMessagesPackResp.hpp"
#include "../messages/CountMessagesResp.hpp"

namespace eyeline {
namespace informer {
namespace dcp {
namespace client {

void DcpClient::assignHandler(DcpClientHandler* newHandler)
{
  handler=newHandler;
}

void DcpClient::decodeAndHandleMessage(const char* buf,size_t sz,int connId)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss,connId);
}

void DcpClient::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss,int connId)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_OkResponse:
    {
      messages::OkResponse msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_FailResponse:
    {
      messages::FailResponse msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetUserStatsResp:
    {
      messages::GetUserStatsResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CreateDeliveryResp:
    {
      messages::CreateDeliveryResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_AddDeliveryMessagesResp:
    {
      messages::AddDeliveryMessagesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryGlossaryResp:
    {
      messages::GetDeliveryGlossaryResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryStateResp:
    {
      messages::GetDeliveryStateResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryInfoResp:
    {
      messages::GetDeliveryInfoResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveriesListResp:
    {
      messages::GetDeliveriesListResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveriesListNextResp:
    {
      messages::GetDeliveriesListNextResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CountDeliveriesResp:
    {
      messages::CountDeliveriesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetDeliveryHistoryResp:
    {
      messages::GetDeliveryHistoryResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_RequestMessagesStateResp:
    {
      messages::RequestMessagesStateResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_GetNextMessagesPackResp:
    {
      messages::GetNextMessagesPackResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_CountMessagesResp:
    {
      messages::CountMessagesResp msg;
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
}
