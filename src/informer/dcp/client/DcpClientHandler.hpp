#ifndef __EYELINE_INFORMER_DCP_CLIETN_DCPCLIENTHANDLER_HPP__
#define __EYELINE_INFORMER_DCP_CLIETN_DCPCLIENTHANDLER_HPP__

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
#include "../messages/RequestMessagesStateResp.hpp"
#include "../messages/GetNextMessagesPackResp.hpp"
#include "../messages/CountMessagesResp.hpp"
#include "../messages/GetDeliveryHistoryResp.hpp"


namespace eyeline{
namespace informer{
namespace dcp{
namespace client{

class DcpClientHandler{
public:
  virtual void handle(const messages::OkResponse& msg)=0;
  virtual void handle(const messages::FailResponse& msg)=0;
  virtual void handle(const messages::GetUserStatsResp& msg)=0;
  virtual void handle(const messages::CreateDeliveryResp& msg)=0;
  virtual void handle(const messages::AddDeliveryMessagesResp& msg)=0;
  virtual void handle(const messages::GetDeliveryGlossaryResp& msg)=0;
  virtual void handle(const messages::GetDeliveryStateResp& msg)=0;
  virtual void handle(const messages::GetDeliveryInfoResp& msg)=0;
  virtual void handle(const messages::GetDeliveriesListResp& msg)=0;
  virtual void handle(const messages::GetDeliveriesListNextResp& msg)=0;
  virtual void handle(const messages::CountDeliveriesResp& msg)=0;
  virtual void handle(const messages::GetDeliveryHistoryResp& msg)=0;
  virtual void handle(const messages::RequestMessagesStateResp& msg)=0;
  virtual void handle(const messages::GetNextMessagesPackResp& msg)=0;
  virtual void handle(const messages::CountMessagesResp& msg)=0;
};

}
}
}
}

#endif
