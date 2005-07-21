#ifndef SCAG_TRANSPORT_SMPP_ACCEPTOR
#define SCAG_TRANSPORT_SMPP_ACCEPTOR

#include "core/threads/ThreadedTask.hpp"
#include "SmeSocket.h"
#include <string>
#include "util/Exception.hpp"
#include "SmppSMInterface.h"

namespace scag {
namespace transport {
namespace smpp {

class SmeAcceptor:public thr::ThreadedTask{
public:
  SmeAcceptor(SmppSMInterface* argSm):sm(argSm){}
  const char* taskName(){return "SmeAcceptor";}
  void Init(const char* host,int port)
  {
    if(sock.InitServer(host,port,0)==-1)
    {
      throw smsc::util::Exception("Failed to init listener at %s:%d",host,port);
    }
    if(sock.StartServer()==-1)
    {
      throw smsc::util::Exception("Failed to listen at %s:%d",host,port);
    }
  }
  int Execute()
  {
    while(!isStopping)
    {
      net::Socket* s=sock.Accept();
      if(!s)break;
      sm->registerSocket(new SmeSocket(s));
    }
    return 0;
  }
  void Stop()
  {
    sock.Close();
    isStopping=true;
  }
protected:
  net::Socket sock;
  SmppSMInterface* sm;
};


}//smpp
}//transport
}//scag

#endif // SCAG_TRANSPORT_SMPP_ACCEPTOR
