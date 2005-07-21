#include "SmppWriter.h"


namespace scag{
namespace transport{
namespace smpp{

int SmppWriter::Execute()
{
  net::Multiplexer::SockArray ready,error;
  while(!isStopping)
  {
    {
      MutexGuard mg(mon);
      while(sockets.Count()==0)
      {
        mon.wait(2000);
        if(isStopping)break;
      }
      if(isStopping)break;
      mul.clear();
      bool haveData=false;
      for(int i=0;i<sockets.Count();i++)
      {
        if(sockets[i]->wantToSend())
        {
          mul.add(sockets[i]->getSocket());
          haveData=true;
        }
      }
      if(!haveData)
      {
        mon.wait(500);
        continue;
      }
    }
    if(mul.canWrite(ready,error,500))
    {
      for(int i=0;i<error.Count();i++)
      {
        getSmppSocket(error[i])->disconnect();
      }
      for(int i=0;i<ready.Count();i++)
      {
        getSmppSocket(ready[i])->sendData();
      }
      MutexGuard mg(mon);
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
