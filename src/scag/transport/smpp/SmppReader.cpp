#include "SmppReader.h"


namespace scag{
namespace transport{
namespace smpp{

int SmppReader::Execute()
{
  net::Multiplexer::SockArray ready;
  net::Multiplexer::SockArray error;
  while(!isStopping)
  {
    debug1(log,"rdtick");
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
      debug2(log,"err=%d, ready=%d",error.Count(),ready.Count());
      for(int i=0;i<error.Count();i++)
      {
        debug2(log,"error on socket %p",error[i]);
        getSmppSocket(error[i])->disconnect();
      }
      for(int i=0;i<ready.Count();i++)
      {
        getSmppSocket(ready[i])->processInput();
      }
      deleteDisconnected();
    }
  }
  return 0;
}

}//smpp
}//transport
}//scag
