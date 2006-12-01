#ifndef __SCAG_TRANSPORT_SMPP_SMSCCONNECTOR_H__
#define __SCAG_TRANSPORT_SMPP_SMSCCONNECTOR_H__

#include <utility>
#include <string>
#include <time.h>
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
  SmscConnector(SmppSMInterface* argSm):sm(argSm),active(true)
  {
    log=smsc::logger::Logger::getInstance("smpp.conn");    
  }

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
  smsc::logger::Logger* log;
  thr::ThreadPool tp;
  SmppSMInterface* sm;
  buf::Hash<SmscConnectInfo> smscConnections;
  sync::Mutex mtx;
  bool active;
};

struct SmscConnectTask:thr::ThreadedTask{
  SmscConnectTask(SmscConnector* argConn,const SmscConnectInfo& info):conn(argConn)
  {
    regSysId=info.regSysId;
    sysId=info.sysId;
    pass=info.pass;
    host=info.host();
    port=info.port();
    addressRange=info.addressRange;
    systemType=info.systemType;
    lastFailure=info.lastFailure;
  }
  const char* taskName(){return "SmscConnectTask";}
  int Execute()
  {
    if(time(NULL)-lastFailure<10)
    {
      timespec ts;
      ts.tv_sec=10;
      ts.tv_nsec=0;
      nanosleep(&ts,0);
    }
    std::auto_ptr<SmscSocket> sock(new SmscSocket(host.c_str(),port));
    if(!sock->connect())
    {
      conn->reportSmscDisconnect(regSysId.c_str());
      return 0;
    }
    sock->bind(regSysId.c_str(),sysId.c_str(),pass.c_str(),addressRange.c_str(),systemType.c_str());
    conn->registerSocket(sock.release());
    return 0;
  }
protected:
  SmscConnector* conn;
  std::string regSysId;
  std::string sysId;
  std::string pass;
  std::string host;
  int port;
  std::string addressRange;
  std::string systemType;
  time_t lastFailure;
};


}//smpp
}//transport
}//scag


#endif
