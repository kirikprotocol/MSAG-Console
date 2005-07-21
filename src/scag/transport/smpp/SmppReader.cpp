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
    MutexGuard mg(mon);
    while(sockets.Count()==0)
    {
      mon.wait(2000);
      if(isStopping)break;
    }
    if(isStopping)break;
    if(mul.canRead(ready,error,200))
    {
      for(int i=0;i<error.Count();i++)
      {
        getSmppSocket(error[i])->disconnect();
      }
      for(int i=0;i<ready.Count();i++)
      {
        getSmppSocket(error[i])->processInput();
      }
      for(int i=sockets.Count()-1;i>=0;i--)
      {
        if(!sockets[i]->isConnected())
        {
          deleteSocket(i);
        }
      }
    }
  }
  return 0;
}

}//smpp
}//transport
}//scag
