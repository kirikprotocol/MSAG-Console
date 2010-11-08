#ifndef __EYELINE_INFORMER_DCP_CLIENT_DCPPROTOCOLCLIENT_HPP__
#define __EYELINE_INFORMER_DCP_CLIENT_DCPPROTOCOLCLIENT_HPP__

#include <string>
#include <vector>
#include "eyeline/protogen/client/ClientBase.hpp"
#include "DcpClientHandler.hpp"
#include "DcpClient.hpp"
#include "../messages/GetUserStatsResp.hpp"
#include "../messages/DeliveryInfo.hpp"
#include "../messages/DeliveryState.hpp"
#include "../messages/DeliveryGlossary.hpp"
#include "../messages/DeliveryListInfo.hpp"
#include "../messages/GetDeliveriesList.hpp"
#include "../messages/CountMessages.hpp"
#include "../messages/MessageInfo.hpp"
#include "../messages/RequestMessagesState.hpp"
#include "../messages/CountDeliveries.hpp"
#include "core/synchronization/Condition.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace eyeline{
namespace informer{
namespace dcp{
namespace client{

namespace sync=smsc::core::synchronization;

class FailedRequestException:public std::exception{
public:
  FailedRequestException(int32_t argCode,const std::string& argMsg):code(argCode),msg(argMsg)
  {
  }

  ~FailedRequestException()throw()
  {
  }

  const char* what()const throw()
  {
    return msg.c_str();
  }
protected:
  std::string msg;
  int code;
};

class InformerClient:public protogen::ClientBase, public DcpClientHandler{
public:
  InformerClient():ClientBase("dcp")
  {
    log=smsc::logger::Logger::getInstance("dcp.client");
    proto.assignHandler(this);
  }

  void userAuth(const std::string& user,const std::string& pass);
  messages::GetUserStatsResp getUserStats();

  int32_t createDelivery(const messages::DeliveryInfo& info);
  void modifyDelivery(int32_t deliveryId,const messages::DeliveryInfo& info);
  void dropDelivery(int32_t deliveryId);
  messages::DeliveryInfo getDeliveryInfo(int32_t deliveryId);
  void changeDeliveryState(int32_t deliveryId,const messages::DeliveryState& state);
  messages::GetDeliveryStateResp getDeliveryState(int32_t deliveryId);
  int32_t getDeliveriesList(messages::GetDeliveriesList& req);
  bool getDeliveriesListNext(int32_t reqId,std::vector<messages::DeliveryListInfo>& result);
  int32_t countDeliveries(messages::CountDeliveries& req);
  void getDeliveryHistory(int32_t deliveryId,std::vector<messages::DeliveryHistoryItem>& result);

  void addDeliveryMessages(int32_t deliveryId,std::vector<messages::DeliveryMessage>& msgs,std::vector<int64_t>& result);
  void dropDeliveryMessages(int32_t deliveryId,std::vector<int64_t>& ids);

  messages::DeliveryGlossary getDeliveryGlossary(int32_t deliveryId);
  void modifyDeliveryGlossary(int32_t deliveryId,messages::DeliveryGlossary& glossary);

  int32_t requestMessagesState(messages::RequestMessagesState& req);
  bool getNextMessagesPack(int32_t reqId,std::vector<messages::MessageInfo>& result);
  int32_t countMessages(messages::CountMessages& req);


protected:
  DcpClient proto;
  smsc::logger::Logger* log;

  enum RequestType{
    rtBase,rtGetUserStats, rtGetDeliveryInfo, rtGetDeliveryState,
    rtGetDeliveriesListNext, rtAddDeliveryMessages, rtGetDeliveryGlossary,
    rtGetNextMessagesPack, rtGetDeliveryHistory
  };
  struct RequestBase{
    typedef RequestBase base;
    RequestType rt;
    int code;
    std::string error;
    int32_t resultData;
    sync::Condition cnd;

    RequestBase(RequestType argRt=rtBase):rt(argRt),code(0)
    {

    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::CreateDeliveryResp& resp)
    {
      resultData=resp.getDeliveryId();
    }

    void processResponse(const messages::CountDeliveriesResp& resp)
    {
      resultData=resp.getResult();
    }

    void processResponse(const messages::GetDeliveriesListResp& resp)
    {
      resultData=resp.getReqId();
    }

    void processResponse(const messages::RequestMessagesStateResp& resp)
    {
      resultData=resp.getReqId();
    }

    void processResponse(const messages::CountMessagesResp& resp)
    {
      resultData=resp.getCount();
    }

  };

  struct RequestGetUserStats:RequestBase{
    RequestGetUserStats():RequestBase(rtGetUserStats)
    {
    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetUserStatsResp& resp)
    {
      result=resp;
    }

    messages::GetUserStatsResp result;
  };


  struct RequestGetDeliveryGlossary:RequestBase{
    RequestGetDeliveryGlossary():
      RequestBase(rtGetDeliveryGlossary)
    {
    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetDeliveryGlossaryResp& resp)
    {
      result=resp.getGlossary();
    }


    messages::DeliveryGlossary result;
  };

  struct RequestGetDeliveryHistory:RequestBase{
    RequestGetDeliveryHistory(std::vector<messages::DeliveryHistoryItem>& argResult):
      RequestBase(rtGetDeliveryHistory),result(argResult)
    {
    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetDeliveryHistoryResp& resp)
    {
      result=resp.getHistory();
    }
    std::vector<messages::DeliveryHistoryItem>& result;
  };

  struct RequestGetDeliveryState:RequestBase{
    RequestGetDeliveryState():
      RequestBase(rtGetDeliveryState)
    {
    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetDeliveryStateResp& resp)
    {
      result=resp;
    }
    messages::GetDeliveryStateResp result;
  };

  struct RequestGetDeliveryInfo:RequestBase{
    RequestGetDeliveryInfo():
      RequestBase(rtGetDeliveryInfo)
    {
    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetDeliveryInfoResp& resp)
    {
      result=resp.getInfo();
    }
    messages::DeliveryInfo result;
  };


  struct RequestGetDeliveriesListNext:RequestBase{
    RequestGetDeliveriesListNext(std::vector<messages::DeliveryListInfo>& argResult):
      RequestBase(rtGetDeliveriesListNext),result(argResult)
    {

    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetDeliveriesListNextResp& resp)
    {
      result=resp.getInfo();
      mms=resp.getMoreDeliveries();
    }

    bool mms;
    std::vector<messages::DeliveryListInfo>& result;
  };

  struct RequestAddDeliveryMessages:RequestBase{
    RequestAddDeliveryMessages(std::vector<int64_t>& argResult):
      RequestBase(rtAddDeliveryMessages),result(argResult)
    {

    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::AddDeliveryMessagesResp& resp)
    {
      result=resp.getMessageIds();
    }

    std::vector<int64_t>& result;
  };

  struct RequestGetNextMessagesPack:RequestBase{
    RequestGetNextMessagesPack(std::vector<messages::MessageInfo>& argResult):
      RequestBase(rtGetNextMessagesPack),result(argResult)
    {

    }

    template <class T> void processResponse(const T&){abort();}

    void processResponse(const messages::GetNextMessagesPackResp& resp)
    {
      result=resp.getInfo();
      mms=resp.getMoreMessages();
    }

    bool mms;
    std::vector<messages::MessageInfo>& result;
  };


  sync::Mutex reqMtx;
  typedef std::map<int,RequestBase*> ReqMap;
  ReqMap reqMap;

  template <class MSG>
  void sendReq(MSG& msg,RequestBase* req)
  {
    sync::MutexGuard mg(reqMtx);
    msg.messageSetSeqNum(seqNum++);
    enqueueMessage(msg,proto);
    reqMap.insert(ReqMap::value_type(msg.messageGetSeqNum(),req));
    req->cnd.WaitOn(reqMtx);
    if(req->code!=0)
    {
      throw FailedRequestException(req->code,req->error);
    }
  }

  void registerOkResp(int seqNum);
  void registerFailResp(int seqNum,int code,const std::string& err);

  template <class MSG>
  void handleResponse(const MSG& msg,RequestType rt)
  {
    sync::MutexGuard mg(reqMtx);
    ReqMap::iterator it=reqMap.find(msg.messageGetSeqNum());
    if(it==reqMap.end())
    {
      smsc_log_warn(log,"request for incoming resp message %s with seqNum=%d not found",msg.messageGetName().c_str(),msg.messageGetSeqNum());
      return;
    }
    if(it->second->rt!=rt)
    {
      smsc_log_warn(log,"received %s, but original request was %s",msg.messageGetName().c_str(),getRequestName(it->second->rt));
      return;
    }
    RequestBase* req=it->second;
    switch(rt)
    {
      case rtBase:req->processResponse(msg);break;
      case rtGetUserStats:((RequestGetUserStats*)req)->processResponse(msg);break;
      case rtGetDeliveryInfo:((RequestGetDeliveryInfo*)req)->processResponse(msg);break;
      case rtGetDeliveryState:((RequestGetDeliveryState*)req)->processResponse(msg);break;
      case rtGetDeliveriesListNext:((RequestGetDeliveriesListNext*)req)->processResponse(msg);break;
      case rtAddDeliveryMessages:((RequestAddDeliveryMessages*)req)->processResponse(msg);break;
      case rtGetDeliveryGlossary:((RequestGetDeliveryGlossary*)req)->processResponse(msg);break;
      case rtGetNextMessagesPack:((RequestGetNextMessagesPack*)req)->processResponse(msg);break;
      case rtGetDeliveryHistory:((RequestGetDeliveryHistory*)req)->processResponse(msg);break;
    }
    it->second->cnd.Signal();
    reqMap.erase(it);
  }


  void onHandleCommand(Buffer& buf);
  void onConnect();
  void onDisconnect();


  void handle(const messages::OkResponse& msg);
  void handle(const messages::FailResponse& msg);
  void handle(const messages::GetUserStatsResp& msg);
  void handle(const messages::CreateDeliveryResp& msg);
  void handle(const messages::AddDeliveryMessagesResp& msg);
  void handle(const messages::GetDeliveryGlossaryResp& msg);
  void handle(const messages::GetDeliveryStateResp& msg);
  void handle(const messages::GetDeliveryInfoResp& msg);
  void handle(const messages::GetDeliveriesListResp& msg);
  void handle(const messages::GetDeliveriesListNextResp& msg);
  void handle(const messages::CountDeliveriesResp& msg);
  void handle(const messages::GetDeliveryHistoryResp& msg);
  void handle(const messages::RequestMessagesStateResp& msg);
  void handle(const messages::GetNextMessagesPackResp& msg);
  void handle(const messages::CountMessagesResp& msg);

  static const char* getRequestName(RequestType rt)
  {
    switch(rt)
    {
      case rtBase:return "Basic";
      case rtGetUserStats:return "GetUserStats";
      case rtGetDeliveryInfo:return "GetDeliveryInfo";
      case rtGetDeliveryState:return "GetDeliveryState";
      case rtGetDeliveriesListNext:return "GetDeliveriesListNext";
      case rtAddDeliveryMessages:return "AddDeliveryMessages";
      case rtGetDeliveryGlossary:return "GetDeliveryGlossary";
      case rtGetNextMessagesPack:return "GetNextMessagesPack";
      case rtGetDeliveryHistory:return "GetDeliveryHistory";
    }
  }

};

}
}
}
}

#endif
