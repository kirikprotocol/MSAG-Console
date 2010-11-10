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
  DcpServer():ServerBase("dcp")
  {
    proto.assignHandler(this);
  }
  void assignCore(InfosmeCore* argCore)
  {
    core=argCore;
  }
protected:

  InfosmeCore* core;
  DcpServerProto proto;

  struct UserConnInfo{
    UserConnInfo():connCount(0){}
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
    mkFailResponse(msg.messagesGetConnId(),msg.messageGetSeqNum(),status,txt);
  }
  template <class T,class U>
  void enqueueResp(T& respMsg,const U& reqMsg)
  {
    respMsg.messageSetSeqNum(reqMsg.messageGetSeqNum());
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

};

}
}
}

#endif
