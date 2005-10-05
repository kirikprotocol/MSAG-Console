#ifndef __SMSC_ADMIN_HSDAEMON_INTERCONNECT_H__
#define __SMSC_ADMIN_HSDAEMON_INTERCONNECT_H__

#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include <string>
#include "util/Exception.hpp"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "Service.h"

namespace smsc{
namespace admin{
namespace hsdaemon{

namespace net=smsc::core::network;
namespace thr=smsc::core::threads;
namespace sync=smsc::core::synchronization;
using smsc::util::Exception;

class Interconnect:public thr::Thread{
public:
  Interconnect(const std::string& argHost,int argPort,const std::string& argOtherHost,int argOtherPort)
  {
    host=argHost;
    port=argPort;
    otherHost=argOtherHost;
    otherPort=argOtherPort;
    running=false;
    log=smsc::logger::Logger::getInstance("hsd.intc");
  }
  void Init()
  {
    if(srvSck.InitServer(host.c_str(),port,0)==-1)
    {
      throw Exception("Failed to init interconnect server socket at %s:%d",host.c_str(),port);
    }
    if(srvSck.StartServer()==-1)
    {
      throw Exception("Failed to start interconnect server socket at %s:%d",host.c_str(),port);
    }
    if(clntSck.Init(otherHost.c_str(),otherPort,0)==-1)
    {
      throw Exception("Failed to init interconnect client socket at %s:%d",otherHost.c_str(),otherPort);
    }
  }
  int Execute();

  void sendSyncCommand();
  Service::run_status remoteGetServiceStatus(const char* svc);
  bool remoteStartService(const char* svc);

protected:
  net::Socket srvSck;
  net::Socket clntSck;
  sync::Mutex clntMtx;
  std::string host;
  int         port;
  std::string otherHost;
  int         otherPort;
  bool        running;
  smsc::logger::Logger* log;

  void ProcessRequest(net::Socket* sck);

  enum Commands{
    cmdSynchronizeServices=1,
    cmdGetServiceStatus,
    cmdStartService
  };
};


extern Interconnect* icon;


}
}
}

#endif
