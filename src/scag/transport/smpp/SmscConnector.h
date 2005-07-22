#ifndef __SCAG_TRANSPORT_SMPP_SMSCCONNECTOR_H__
#define __SCAG_TRANSPORT_SMPP_SMSCCONNECTOR_H__

#include <utility>
#include <string>
#include "core/threads/ThreadPool.hpp"
#include "SmppSMInterface.h"
#include "SmscSocket.h"
#include "SmscConnectorAdmin.h"

namespace scag{
namespace transport{
namespace smpp{

namespace thr=smsc::core::threads;


class SmscConnector:public SmscConnectorAdmin{
public:
  SmscConnector(SmppSMInterface* argSm):sm(argSm),active(true){}

  void addSmscConnect(const SmscConnectInfo& info);
  void deleteSmscConnect(const char* sysId);
  void updateSmscConnect(const SmscConnectInfo& info);

  void reportSmscDisconnect(const char* sysId);
  void registerSocket(SmscSocket* sock)
  {
    sm->registerSocket(sock);
  }
  void shutdown()
  {
    active=false;
  }
protected:
  thr::ThreadPool tp;
  SmppSMInterface* sm;
  buf::Hash<SmscConnectInfo> smscConnections;
  sync::Mutex mtx;
  bool active;
};

struct SmscConnectTask:thr::ThreadedTask{
  SmscConnectTask(SmscConnector* argConn,const SmscConnectInfo& info):conn(argConn)
  {
    sysId=info.sysId;
    pass=info.pass;
    host=info.host();
    port=info.port();
  }
  const char* taskName(){return "SmscConnectTask";}
  int Execute()
  {
    std::auto_ptr<SmscSocket> sock(new SmscSocket(host.c_str(),port));
    if(!sock->connect())
    {
      conn->reportSmscDisconnect(sysId.c_str());
      return 0;
    }
    sock->bind(sysId.c_str(),pass.c_str());
    conn->registerSocket(sock.release());
    return 0;
  }
protected:
  SmscConnector* conn;
  std::string sysId;
  std::string pass;
  std::string host;
  int port;
};


}//smpp
}//transport
}//scag


#endif
