#ifndef __EYELINE_INFORMER_DCP_DCPSERVER_HPP__
#define __EYELINE_INFORMER_DCP_DCPSERVER_HPP__

#include "core/synchronization/Mutex.hpp"
#include "eyeline/protogen/server/ServerBase.hpp"
#include "informer/dcp/DcpHandler.hpp"
#include "DcpServerProto.hpp"
#include "informer/data/InfosmeCore.h"
#include "DcpError.hpp"

namespace eyeline{
namespace informer{
namespace dcp{

namespace net=smsc::core::network;
namespace sync=smsc::core::synchronization;

class DcpClientSocket:public eyeline::protogen::ProtocolSocketBase{
public:
  DcpClientSocket(net::Socket* sck,int connId):ProtocolSocketBase(sck,connId),authorized(false)
  {
  }
  virtual ~DcpClientSocket(){}
  void setAuthorized(bool value)
  {
    authorized=value;
  }
  bool isAuthorized()const
  {
    return authorized;
  }
  void setUserInfo(UserInfoPtr argUI)
  {
    ui=argUI;
  }
  UserInfoPtr& getUserInfo()
  {
    return ui;
  }
protected:
  bool authorized;
  UserInfoPtr ui;
};

class DcpServer:public eyeline::protogen::ServerBase,public DcpHandler{
public:
  DcpServer():ServerBase("dcp.proto")
  {
    proto.assignHandler(this);
    dlvListReqIdSeq=0;
    dlvListReqExpirationTime=3600;
    dumpLog=smsc::logger::Logger::getInstance("dcp.msgdmp");
  }
  virtual ~DcpServer()
  {

  }
  void assignCore(InfosmeCore* argCore)
  {
    core=argCore;
  }
protected:

  InfosmeCore* core;
  DcpServerProto proto;

  smsc::logger::Logger* dumpLog;

  template <class MSG>
  void dumpMsg(const MSG& msg,const char* dir="in")
  {
    if(dumpLog->isDebugEnabled())
    {
      const std::string& dumpStr=msg.toString();
      if(dumpStr.length()>256)
      {
        smsc_log_debug(dumpLog,"%s connId=%d, %s:%s ...",dir,msg.messageGetConnId(),msg.messageGetName().c_str(),dumpStr.substr(0,256).c_str());
      }else
      {
        smsc_log_debug(dumpLog,"%s connId=%d, %s:%s",dir,msg.messageGetConnId(),msg.messageGetName().c_str(),dumpStr.c_str());
      }
    }
  }

  struct UserConnInfo{
    UserConnInfo(int argConnCount=0):connCount(argConnCount){}
    int connCount;
  };

  typedef std::map<std::string,UserConnInfo> UsersMap;
  UsersMap usersMap;
  sync::Mutex usersMtx;

  //server base
  virtual eyeline::protogen::ProtocolSocketBase* onConnect(net::Socket* clnt,int connId);
  virtual void onHandleCommand(eyeline::protogen::ProtocolSocketBase::Packet& pkt);
  virtual void onDisconnect(eyeline::protogen::ProtocolSocketBase* sck);

  //dcp protocol handler
  void handle(const messages::UserAuth& inmsg);
  void handle(const messages::GetUserStats& inmsg);
  void handle(const messages::CreateDelivery& inmsg);
  void handle(const messages::ModifyDelivery& inmsg);
  void handle(const messages::DropDelivery& inmsg);
  void handle(const messages::ChangeDeliveryState& inmsg);
  void handle(const messages::AddDeliveryMessages& inmsg);
  void handle(const messages::DropDeliveryMessages& inmsg);
  void handle(const messages::GetDeliveryGlossary& inmsg);
  void handle(const messages::ModifyDeliveryGlossary& inmsg);
  void handle(const messages::GetDeliveryState& inmsg);
  void handle(const messages::GetDeliveryInfo& inmsg);
  void handle(const messages::GetDeliveriesList& inmsg);
  void handle(const messages::GetDeliveriesListNext& inmsg);
  void handle(const messages::CountDeliveries& inmsg);
  void handle(const messages::RequestMessagesState& inmsg);
  void handle(const messages::GetNextMessagesPack& inmsg);
  void handle(const messages::CountMessages& inmsg);
  void handle(const messages::GetDeliveryHistory& inmsg);


  // aux methods
  void mkOkResponse(int connId,int seqNum);
  template <class MSG>
  void mkOkResponse(const MSG& msg)
  {
    mkOkResponse(msg.messageGetConnId(),msg.messageGetSeqNum());
  }
  void mkFailResponse(int connId,int seqNum,int status,const std::string& msg);
  template <class MSG>
  void mkFailResponse(const MSG& msg,int status,const std::string& txt)
  {
    mkFailResponse(msg.messageGetConnId(),msg.messageGetSeqNum(),status,txt);
  }
  template <class T,class U>
  void enqueueResp(T& respMsg,const U& reqMsg)
  {
    respMsg.messageSetSeqNum(reqMsg.messageGetSeqNum());
    respMsg.messageSetConnId(reqMsg.messageGetConnId());
    dumpMsg(respMsg,"out");
    enqueueCommand(reqMsg.messageGetConnId(),respMsg,proto,false);
  }

  template <class MSG>
  bool checkAuth(DcpClientSocket* sck,const MSG& msg)
  {
    if(!sck->isAuthorized())
    {
      mkFailResponse(sck->getConnId(),msg.messageGetSeqNum(),DcpError::NotAuthorized,"connection not authorized");
      return false;
    }
    return true;
  }


  template <class MSG>
  bool checkAdmin(DcpClientSocket* sck,const MSG& msg)
  {
    if(!sck->getUserInfo()->hasRole(USERROLE_ADMIN))
    {
      mkFailResponse(sck->getConnId(),msg.messageGetSeqNum(),DcpError::AdminRoleRequired,"admin role required for this command");
      return false;
    }
    return true;
  }


  template <class MSG>
  UserInfoPtr getUserInfo(const MSG& msg)
  {
    sync::MutexGuard mg(clntsMon);
    DcpClientSocket* sck=getSocketByConnId(msg.messageGetConnId());
    if(!checkAuth(sck,msg))
    {
      throw InfosmeException(EXC_NOTAUTH,"Not authorized");
    }
    return sck->getUserInfo();
  }

  inline InfosmeCore* getCore() const {
      if (!core) {
          throw InfosmeException(EXC_SYSTEM,"Not ready yet");
      }
      return core;
  }

  /*
  template <class MSG>
  UserInfoPtr getUserInfoAdmin(const MSG& msg)
  {
    sync::MutexGuard mg(clntsMon);
    DcpClientSocket* sck=getSocketByConnId(msg.messageGetConnId());
    if(!checkAuth(sck,msg))
    {
      return 0;
    }
    if(msg.hasUserId() && !checkAdmin(sck,msg))
    {
      return 0;
    }
    if(msg.hasUserId())
    {
      return core->getUserInfo(msg.getUserId().c_str());
    }else
    {
      return sck->getUserInfo();
    }
  }
*/

  DcpClientSocket* getSocketByConnId(int connId);

  typedef std::multimap<time_t,int> DlvListReqTimeMap;

  struct DlvListRequest{
    DlvListRequest(int connId,int argReqId):reqId(argReqId),last(0)
    {

    }
    int connId;
    int reqId;
    std::vector<messages::DeliveryFields> fields;
    std::vector<UserInfoPtr>    userLst;
    dlvid_type                  last;
    messages::DeliveriesFilter  filter;
    DlvListReqTimeMap::iterator timeMapIt;
  };
  sync::Mutex dlvReqMtx;
  typedef std::map<int,DlvListRequest*> DlvListReqMap;
  DlvListReqMap dlvListReqMap;
  DlvListReqTimeMap dlvListReqTimeMap;
  int dlvListReqIdSeq;
  time_t dlvListReqExpirationTime;

};

}
}
}

#endif
