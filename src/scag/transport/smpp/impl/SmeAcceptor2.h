#ifndef SCAG_TRANSPORT_SMPP_ACCEPTOR2
#define SCAG_TRANSPORT_SMPP_ACCEPTOR2

#include <string>

#include "SmeSocket2.h"
#include "scag/transport/smpp/base/SmppSMInterface2.h"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"
#include "util/Exception.hpp"

namespace scag2 {
namespace transport {
namespace smpp {

    using namespace smsc::core::threads;
    using namespace smsc::core::network;

class SmeAcceptor: public ThreadedTask
{
public:
  SmeAcceptor(SmppSMInterface* argSm):sm(argSm), isStopped_(false)
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
        if (isStopped_) return 0;
        while(!isStopping)
        {
            smsc_log_debug(log,"prior to accept");
            Socket* s=sock.Accept();
            if(!s)break;
            char buf[32];
            s->GetPeer(buf);
            SmppSocketPtr sockPtr(new SmeSocket(s));
            SmppSocket* sockp = sockPtr.get();
            smsc_log_info(log,"incoming connection %p from %s sock=%p",sockp,buf,s);
            try {
                if ( sm->registerSocket(sockp) ) {
                    smsc_log_info(log,"connection %p %s sock=%p accepted",sockp,buf,s);
                } else {
                    smsc_log_info(log,"connection %p %s sock=%p rejected",sockp,buf,s);
                }
            } catch ( std::exception& e ) {
                smsc_log_error(log,"exc in registerSocket(%p,%s,sock=%p): %s",sockp,buf,s,e.what());
            } catch (...) {
                smsc_log_error(log,"exc in registerSocket(%p,%s,sock=%p): unknown",sockp,buf,s);
            }
        }
        smsc_log_info(log,"SmeAcceptor stopped");
        isStopped_ = true;
        return 0;
    }
  virtual void stop()
  {
    smsc_log_info(log, "SmeAcceptor stopping");
    sock.Close();
    isStopping=true;
  }
  bool isStopped() const { return isStopped_; }
protected:
  Socket sock;
  SmppSMInterface* sm;
  smsc::logger::Logger* log;
  bool isStopped_;
};


}//smpp
}//transport
}//scag

#endif // SCAG_TRANSPORT_SMPP_ACCEPTOR
