#ifndef __EYELINE_INFORMER_DCP_DCPSERVERPROTO_HPP__
#define __EYELINE_INFORMER_DCP_DCPSERVERPROTO_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DcpHandler.hpp"
#include "messages/OkResponse.hpp"
#include "messages/FailResponse.hpp"
#include "messages/GetUserStatsResp.hpp"
#include "messages/CreateDeliveryResp.hpp"
#include "messages/AddDeliveryMessagesResp.hpp"
#include "messages/GetDeliveryGlossaryResp.hpp"
#include "messages/GetDeliveryStateResp.hpp"
#include "messages/GetDeliveryInfoResp.hpp"
#include "messages/GetDeliveriesListResp.hpp"
#include "messages/GetDeliveriesListNextResp.hpp"
#include "messages/CountDeliveriesResp.hpp"
#include "messages/GetDeliveryHistoryResp.hpp"
#include "messages/RequestMessagesStateResp.hpp"
#include "messages/GetNextMessagesPackResp.hpp"
#include "messages/CountMessagesResp.hpp"

namespace eyeline {
namespace informer {
namespace dcp {

class DcpServerProto{
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
 
  DcpServerProto():handler(0)
  {
  }

  void assignHandler(DcpHandler* newHandler);
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
  DcpHandler* handler;
};

}
}
}

#endif
