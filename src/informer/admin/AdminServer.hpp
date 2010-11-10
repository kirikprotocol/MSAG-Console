#ifndef __EYELINE_INFORMER_ADMIN_ADMINSERVER_HPP__
#define __EYELINE_INFORMER_ADMIN_ADMINSERVER_HPP__

#include "AdminCommandsHandler.hpp"
#include "eyeline/protogen/server/ServerBase.hpp"
#include "InformerAdminProto.hpp"
#include "informer/data/InfosmeCore.h"
#include "logger/Logger.h"


namespace eyeline{
namespace informer{
namespace admin{

namespace net=smsc::core::network;

class AdminServer:public eyeline::protogen::ServerBase,public AdminCommandsHandler{
public:
  AdminServer():ServerBase("admin")
  {
    log=smsc::logger::Logger::getInstance("admin");
    proto.assignHandler(this);
  }
  void assignCore(InfosmeCore* argCore)
  {
    core=argCore;
  }
protected:

  InformerAdminProto proto;

  InfosmeCore* core;

  smsc::logger::Logger* log;

  virtual eyeline::protogen::ProtocolSocketBase* onConnect(net::Socket* clnt,int connId);
  virtual void onHandleCommand(eyeline::protogen::ProtocolSocketBase::Packet& pkt);
  virtual void onDisconnect(eyeline::protogen::ProtocolSocketBase* sck);

  virtual void handle(const messages::ConfigOp& cmd);
  virtual void handle(const messages::SetDefaultSmsc& cmd);
  virtual void handle(const messages::LoggerGetCategories& cmd);
  virtual void handle(const messages::LoggerSetCategories& cmd);
  virtual void handle(const messages::SendTestSms& cmd);

};


}
}
}


#endif
