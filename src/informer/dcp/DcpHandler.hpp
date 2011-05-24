#ifndef __EYELINE_INFORMER_DCP_DCPHANDLER_HPP__
#define __EYELINE_INFORMER_DCP_DCPHANDLER_HPP__

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

#include "messages/RequestMessagesState.hpp"
#include "messages/GetNextMessagesPack.hpp"
#include "messages/GetDeliveryHistory.hpp"
#include "messages/CountMessages.hpp"
#include "messages/CountMessagesPack.hpp"


namespace eyeline{
namespace informer{
namespace dcp{

class DcpHandler{
public:
  virtual void handle(const messages::UserAuth& inmsg)=0;
  virtual void handle(const messages::GetUserStats& inmsg)=0;
  virtual void handle(const messages::CreateDelivery& inmsg)=0;
  virtual void handle(const messages::ModifyDelivery& inmsg)=0;
  virtual void handle(const messages::DropDelivery& inmsg)=0;
  virtual void handle(const messages::ChangeDeliveryState& inmsg)=0;
  virtual void handle(const messages::AddDeliveryMessages& inmsg)=0;
  virtual void handle(const messages::DropDeliveryMessages& inmsg)=0;
  virtual void handle(const messages::GetDeliveryGlossary& inmsg)=0;
  virtual void handle(const messages::ModifyDeliveryGlossary& inmsg)=0;
  virtual void handle(const messages::GetDeliveryState& inmsg)=0;
  virtual void handle(const messages::GetDeliveryInfo& inmsg)=0;
  virtual void handle(const messages::GetDeliveryHistory& inmsg)=0;
  virtual void handle(const messages::GetDeliveriesList& inmsg)=0;
  virtual void handle(const messages::GetDeliveriesListNext& inmsg)=0;
  virtual void handle(const messages::CountDeliveries& inmsg)=0;
  virtual void handle(const messages::CountDeliveriesNext& inmsg)=0;
  virtual void handle(const messages::RequestMessagesState& inmsg)=0;
  virtual void handle(const messages::GetNextMessagesPack& inmsg)=0;
  virtual void handle(const messages::CountMessages& inmsg)=0;
  virtual void handle(const messages::CountMessagesPack& inmsg)=0;

};

}
}
}

#endif
