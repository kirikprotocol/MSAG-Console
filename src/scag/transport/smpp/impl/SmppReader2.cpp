#include "SmppReader2.h"


namespace scag2 {
namespace transport {
namespace smpp {

int SmppReader::Execute()
{
  smsc_log_debug(log_,"Starting SmppReader");
  net::Multiplexer::SockArray ready;
  net::Multiplexer::SockArray error;
  while(!isStopping)
  {
    try{
      {
        MutexGuard mg(mon);
        while(sockets.Count()==0)
        {
          mon.wait(2000);
          if(isStopping)break;
        }
        if(isStopping)break;
      }
      bool haveEvents;
      {
        MutexGuard mg(mulMtx);
        haveEvents=mul.canRead(ready,error,200);
      }
      if(haveEvents)
      {
        MutexGuard mg(mon);
        for(int i=0;i<error.Count();i++)
        {
          smsc_log_warn(log_,"reader:error on socket %p",error[i]);
          getSmppSocket(error[i])->disconnect();
        }
        for(int i=0;i<ready.Count();i++)
        {
          getSmppSocket(ready[i])->processInput();
        }
        deleteDisconnected();
      }
    }catch(std::exception& e)
    {
      smsc_log_warn(log_,"!!! Exception in SmppReader:%s",e.what());
    }
  }
  smsc_log_debug(log_,"Execution of SmppReader finished");
  return 0;
}

}//smpp
}//transport
}//scag
