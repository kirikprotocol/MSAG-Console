#include "InformerClient.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace client{

void InformerClient::registerOkResp(int respSeqNum)
{
  sync::MutexGuard mg(reqMtx);
  ReqMap::iterator it=reqMap.find(respSeqNum);
  if(it==reqMap.end())
  {
    smsc_log_info(log,"request with seqNum=%d not found",respSeqNum);
    return;
  }
  it->second->code=0;
  it->second->cnd.Signal();
  reqMap.erase(it);
}

void InformerClient::registerFailResp(int respSeqNum,int code,const std::string& err)
{
  sync::MutexGuard mg(reqMtx);
  ReqMap::iterator it=reqMap.find(respSeqNum);
  if(it==reqMap.end())
  {
    smsc_log_info(log,"request with seqNum=%d not found",respSeqNum);
    return;
  }
  it->second->code=code;
  it->second->error=err;
  it->second->cnd.Signal();
  reqMap.erase(it);
}


void InformerClient::onHandleCommand(Buffer& buf)
{
  proto.decodeAndHandleMessage(buf.data,buf.dataSize,0);
}

void InformerClient::onConnect()
{
  smsc_log_info(log,"connect");
}

void InformerClient::onDisconnect()
{
  sync::MutexGuard mg(reqMtx);
  while(!reqMap.empty())
  {
    ReqMap::iterator it=reqMap.begin();
    it->second->code=2;
    it->second->error="disconnect";
    it->second->cnd.Signal();
    reqMap.erase(it);
  }
  smsc_log_info(log,"disconnect");
}


void InformerClient::userAuth(const std::string& user,const std::string& pass)
{
  messages::UserAuth msg;
  msg.setUserId(user);
  msg.setPassword(pass);
  RequestBase req;
  sendReq(msg,&req);
}

messages::GetUserStatsResp InformerClient::getUserStats()
{
  messages::GetUserStats msg;
  RequestGetUserStats req;
  sendReq(msg,&req);
  return req.result;
}


int32_t InformerClient::createDelivery(const messages::DeliveryInfo& info)
{
  messages::CreateDelivery msg;
  msg.setInfo(info);
  RequestBase req;
  sendReq(msg,&req);
  return req.resultData;
}


void InformerClient::modifyDelivery(int32_t deliveryId,const messages::DeliveryInfo& info)
{
  messages::ModifyDelivery msg;
  msg.setDeliveryId(deliveryId);
  msg.setInfo(info);
  RequestBase req;
  sendReq(msg,&req);
}


void InformerClient::dropDelivery(int32_t deliveryId)
{
  messages::DropDelivery msg;
  msg.setDeliveryId(deliveryId);
  RequestBase req;
  sendReq(msg,&req);
}


messages::DeliveryInfo InformerClient::getDeliveryInfo(int32_t deliveryId)
{
  messages::GetDeliveryInfo msg;
  msg.setDeliveryId(deliveryId);
  RequestGetDeliveryInfo req;
  sendReq(msg,&req);
  return req.result;
}


void InformerClient::changeDeliveryState(int32_t deliveryId,const messages::DeliveryState& state)
{
  messages::ChangeDeliveryState msg;
  msg.setDeliveryId(deliveryId);
  msg.setState(state);
  RequestBase req;
  sendReq(msg,&req);
}


void InformerClient::addDeliveryMessages(int32_t deliveryId,std::vector<messages::DeliveryMessage>& msgs,std::vector<int64_t>& result)
{
  messages::AddDeliveryMessages msg;
  msg.setDeliveryId(deliveryId);
  msg.getMessagesRef().swap(msgs);
  RequestAddDeliveryMessages req(result);
  sendReq(msg,&req);
}


void InformerClient::dropDeliveryMessages(int32_t deliveryId,std::vector<int64_t>& ids)
{
  messages::DropDeliveryMessages msg;
  msg.setDeliveryId(deliveryId);
  msg.setMessageIds(ids);
  RequestBase req;
  sendReq(msg,&req);
}


messages::DeliveryGlossary InformerClient::getDeliveryGlossary(int32_t deliveryId)
{
  messages::GetDeliveryGlossary msg;
  msg.setDeliveryId(deliveryId);
  RequestGetDeliveryGlossary req;
  sendReq(msg,&req);
  return req.result;
}


void InformerClient::modifyDeliveryGlossary(int32_t deliveryId,messages::DeliveryGlossary& glossary)
{
  messages::ModifyDeliveryGlossary msg;
  msg.setDeliveryId(deliveryId);
  msg.setGlossary(glossary);
  RequestBase req;
  sendReq(msg,&req);
}


messages::GetDeliveryStateResp InformerClient::getDeliveryState(int32_t deliveryId)
{
  messages::GetDeliveryState msg;
  msg.setDeliveryId(deliveryId);
  RequestGetDeliveryState req;
  sendReq(msg,&req);
  return req.result;
}


int32_t InformerClient::getDeliveriesList(messages::GetDeliveriesList& msg)
{
  RequestBase req;
  sendReq(msg,&req);
  return req.resultData;
}


bool InformerClient::getDeliveriesListNext(int32_t reqId,int count,std::vector<messages::DeliveryListInfo>& result)
{
  messages::GetDeliveriesListNext msg;
  msg.setReqId(reqId);
  msg.setCount(count);
  RequestGetDeliveriesListNext req(result);
  sendReq(msg,&req);
  return req.mms;
}


int32_t InformerClient::countDeliveries(messages::CountDeliveries& msg)
{
  RequestBase req;
  sendReq(msg,&req);
  return req.resultData;
}


int32_t InformerClient::requestMessagesState(messages::RequestMessagesState& msg)
{
  RequestBase req;
  sendReq(msg,&req);
  return req.resultData;
}


bool InformerClient::getNextMessagesPack(int32_t reqId,int count,std::vector<messages::MessageInfo>& result)
{
  messages::GetNextMessagesPack msg;
  msg.setReqId(reqId);
  msg.setCount(count);
  RequestGetNextMessagesPack req(result);
  sendReq(msg,&req);
  return req.mms;
}


int32_t InformerClient::countMessages(messages::CountMessages& msg)
{
  RequestBase req;
  sendReq(msg,&req);
  return req.resultData;
}

void InformerClient::getDeliveryHistory(int32_t deliveryId,std::vector<messages::DeliveryHistoryItem>& result)
{
  RequestGetDeliveryHistory req(result);
  messages::GetDeliveryHistory msg;
  msg.setDeliveryId(deliveryId);
  sendReq(msg,&req);
}




void InformerClient::handle(const messages::OkResponse& msg)
{
  registerOkResp(msg.messageGetSeqNum());
}

void InformerClient::handle(const messages::FailResponse& msg)
{
  registerFailResp(msg.messageGetSeqNum(),msg.getStatus(),msg.getStatusMessage());
}

void InformerClient::handle(const messages::GetUserStatsResp& msg)
{
  handleResponse(msg,rtGetUserStats);
}

void InformerClient::handle(const messages::CreateDeliveryResp& msg)
{
  handleResponse(msg,rtBase);
}

void InformerClient::handle(const messages::AddDeliveryMessagesResp& msg)
{
  handleResponse(msg,rtAddDeliveryMessages);
}

void InformerClient::handle(const messages::GetDeliveryGlossaryResp& msg)
{
  handleResponse(msg,rtGetDeliveryGlossary);
}

void InformerClient::handle(const messages::GetDeliveryStateResp& msg)
{
  handleResponse(msg,rtGetDeliveryState);
}

void InformerClient::handle(const messages::GetDeliveryInfoResp& msg)
{
  handleResponse(msg,rtGetDeliveryInfo);
}

void InformerClient::handle(const messages::GetDeliveriesListResp& msg)
{
  handleResponse(msg,rtBase);
}

void InformerClient::handle(const messages::GetDeliveriesListNextResp& msg)
{
  handleResponse(msg,rtGetDeliveriesListNext);
}

void InformerClient::handle(const messages::CountDeliveriesResp& msg)
{
  handleResponse(msg,rtBase);
}

void InformerClient::handle(const messages::CountDeliveriesNextResp& msg)
{
  handleResponse(msg,rtBase);
}

void InformerClient::handle(const messages::GetDeliveryHistoryResp& msg)
{
  handleResponse(msg,rtGetDeliveryHistory);
}


void InformerClient::handle(const messages::RequestMessagesStateResp& msg)
{
  handleResponse(msg,rtBase);
}

void InformerClient::handle(const messages::GetNextMessagesPackResp& msg)
{
  handleResponse(msg,rtGetNextMessagesPack);
}

void InformerClient::handle(const messages::CountMessagesResp& msg)
{
  handleResponse(msg,rtBase);
}

void InformerClient::handle(const messages::CountMessagesPackResp& msg)
{
  handleResponse(msg,rtCountMessagesPack);
}


}
}
}
}
