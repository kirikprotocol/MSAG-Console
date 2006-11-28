#ifndef SCAG_TRANSPORT_SMPP_ACCEPTOR
#define SCAG_TRANSPORT_SMPP_ACCEPTOR

#include "core/threads/ThreadedTask.hpp"
#include "SmeSocket.h"
#include <string>
#include "util/Exception.hpp"
#include "SmppSMInterface.h"
#include "logger/Logger.h"

namespace scag {
namespace transport {
namespace smpp {

namespace thr=smsc::core::threads;

class SmeAcceptor:public thr::ThreadedTask{
public:
  SmeAcceptor(SmppSMInterface* argSm):sm(argSm)
  {
    log=smsc::logger::Logger::getInstance("smpp.acc");
  }
  const char* taskName(){return "SmeAcceptor";}
  void Init(const char* host,int port)
  {
    if(sock.InitServer(host,port,0,1,true)==-1)
    {
      throw smsc::util::Exception("Failed to init listener at %s:%d",host,port);
    }
    if(sock.StartServer()==-1)
    {
      throw smsc::util::Exception("Failed to listen at %s:%d",host,port);
    }
    smsc_log_info(log,"SmeAcceptor inited");      
  }
  int Execute()
  {
    while(!isStopping)
    {
      net::Socket* s=sock.Accept();
      if(!s)break;
      char buf[32];
      s->GetPeer(buf);
      smsc_log_info(log,"connection accepted from %s",buf);
      sm->registerSocket(new SmeSocket(s));
    }
    smsc_log_info(log,"SmeAcceptor stopped");
    return 0;
  }
  void Stop()
  {
    smsc_log_info(log, "SmeAcceptor stopping");
    sock.Close();
    isStopping=true;
  }
protected:
  net::Socket sock;
  SmppSMInterface* sm;
  smsc::logger::Logger* log;
};


}//smpp
}//transport
}//scag

#endif // SCAG_TRANSPORT_SMPP_ACCEPTOR
