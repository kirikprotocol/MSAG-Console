#ifndef __SCAG_TRANSPORT_SMPP_SMSCCONNECTOR2_H__
#define __SCAG_TRANSPORT_SMPP_SMSCCONNECTOR2_H__

#include <utility>
#include <string>
#include <time.h>
#include "core/threads/ThreadPool.hpp"
#include "scag/transport/smpp/base/SmppSMInterface2.h"
#include "SmscSocket2.h"
#include "SmscConnectorAdmin2.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;


class SmscConnector:public SmscConnectorAdmin{
public:
  SmscConnector(SmppSMInterface* argSm):sm(argSm),active(true)
  {
    log=smsc::logger::Logger::getInstance("smpp.conn");
  }

  void Init(const char* argBindHost)
  {
    bindHost=argBindHost;
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
      tp.shutdown();
  }
  const std::string& getBindHost()
  {
    return bindHost;
  }
private:
  smsc::logger::Logger* log;
  ThreadPool tp;
  SmppSMInterface* sm;
  Hash<SmscConnectInfo> smscConnections;
  Mutex mtx;
  bool active;
  std::string bindHost;
};

struct SmscConnectTask : ThreadedTask{
  SmscConnectTask(SmscConnector* argConn,const SmscConnectInfo& info):conn(argConn)
  {
    log=smsc::logger::Logger::getInstance("smpp.conn");
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
    smsc_log_info(log, "Connecting to '%s' (%s:%d)", regSysId.c_str(), host.c_str(), port);
    std::auto_ptr<SmscSocket> sock(new SmscSocket(host.c_str(),port));
    if(!sock->connect(conn->getBindHost()))
    {
      conn->reportSmscDisconnect(regSysId.c_str());
      return 0;
    }
    sock->bind(regSysId.c_str(),sysId.c_str(),pass.c_str(),addressRange.c_str(),systemType.c_str());
    conn->registerSocket(sock.release());
    smsc_log_info(log, "Connected to '%s' (%s:%d)", regSysId.c_str(), host.c_str(), port);
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
  smsc::logger::Logger* log;
};


}//smpp
}//transport
}//scag


#endif
