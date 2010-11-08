#ifndef __EYELINE_INFORMER_DCP_CLIENT_DCPCLIENT_HPP__
#define __EYELINE_INFORMER_DCP_CLIENT_DCPCLIENT_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DcpClientHandler.hpp"
#include "../messages/UserAuth.hpp"
#include "../messages/GetUserStats.hpp"
#include "../messages/CreateDelivery.hpp"
#include "../messages/ModifyDelivery.hpp"
#include "../messages/DropDelivery.hpp"
#include "../messages/ChangeDeliveryState.hpp"
#include "../messages/AddDeliveryMessages.hpp"
#include "../messages/DropDeliveryMessages.hpp"
#include "../messages/GetDeliveryGlossary.hpp"
#include "../messages/ModifyDeliveryGlossary.hpp"
#include "../messages/GetDeliveryState.hpp"
#include "../messages/GetDeliveryInfo.hpp"
#include "../messages/GetDeliveriesList.hpp"
#include "../messages/GetDeliveriesListNext.hpp"
#include "../messages/CountDeliveries.hpp"
#include "../messages/GetDeliveryHistory.hpp"
#include "../messages/RequestMessagesState.hpp"
#include "../messages/GetNextMessagesPack.hpp"
#include "../messages/CountMessages.hpp"

namespace eyeline {
namespace informer {
namespace dcp {
namespace client {

class DcpClient{
public:
  enum{
    tag_OkResponse=1,
    tag_FailResponse=2,
    tag_UserAuth=11,
    tag_GetUserStats=12,
    tag_GetUserStatsResp=13,
    tag_CreateDelivery=30,
    tag_CreateDeliveryResp=31,
    tag_ModifyDelivery=32,
    tag_DropDelivery=33,
    tag_ChangeDeliveryState=34,
    tag_AddDeliveryMessages=35,
    tag_AddDeliveryMessagesResp=36,
    tag_DropDeliveryMessages=37,
    tag_GetDeliveryGlossary=38,
    tag_GetDeliveryGlossaryResp=39,
    tag_ModifyDeliveryGlossary=40,
    tag_GetDeliveryState=41,
    tag_GetDeliveryStateResp=42,
    tag_GetDeliveryInfo=48,
    tag_GetDeliveryInfoResp=49,
    tag_GetDeliveriesList=50,
    tag_GetDeliveriesListResp=51,
    tag_GetDeliveriesListNext=52,
    tag_GetDeliveriesListNextResp=53,
    tag_CountDeliveries=54,
    tag_CountDeliveriesResp=55,
    tag_GetDeliveryHistory=58,
    tag_GetDeliveryHistoryResp=59,
    tag_RequestMessagesState=43,
    tag_RequestMessagesStateResp=44,
    tag_GetNextMessagesPack=45,
    tag_GetNextMessagesPackResp=46,
    tag_CountMessages=56,
    tag_CountMessagesResp=57
  };
 
  DcpClient():handler(0)
  {
  }

  void assignHandler(DcpClientHandler* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz,int connId);
  void decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss,int connId);

  template <class MsgType>
  void encodeMessage(const MsgType& msg,eyeline::protogen::framework::SerializerBuffer* ss)
  {
    ss->writeInt32(4+4+msg.template length<eyeline::protogen::framework::SerializerBuffer>());
    ss->writeInt32(msg.messageGetTag());
    ss->writeInt32(msg.messageGetSeqNum());
    msg.serialize(*ss);
  }

protected:
  DcpClientHandler* handler;
};

}
}
}
}

#endif
