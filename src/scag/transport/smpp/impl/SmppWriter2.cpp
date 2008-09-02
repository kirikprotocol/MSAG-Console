#include "SmppWriter2.h"


namespace scag2 {
namespace transport{
namespace smpp{

int SmppWriter::Execute()
{
  smsc_log_debug(log,"Starting SmppWriter");
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
        if(sockets[i]->checkTimeout(enquireLinkTimeout))
        {
          sockets[i]->genEnquireLink(enquireLinkTimeout);
        }
        if(sockets[i]->checkTimeout(inactivityTimeout))
        {
          smsc_log_warn(log, "SmppWriter: inactivity timeout expired");
          sockets[i]->disconnect();
          continue;
        }
        if(sockets[i]->wantToSend())
        {
          mul.add(sockets[i]->getSocket());
          haveData=true;
        }
      }
      if(!haveData)
      {
        mon.wait(500);
        deleteDisconnected();
        continue;
      }
    }
    if(mul.canWrite(ready,error,500))
    {
      for(int i=0;i<error.Count();i++)
      {
        smsc_log_warn(log,"writer: error on socket %p",error[i]);
        getSmppSocket(error[i])->disconnect();
      }
      for(int i=0;i<ready.Count();i++)
      {
        try{
          getSmppSocket(ready[i])->sendData();
        }catch(std::exception& e)
        {
          smsc_log_warn(log,"exception in sendData:%s",e.what());
        }
      }
      MutexGuard mg(mon);
      deleteDisconnected();
    }
  }
  deleteDisconnected();
  smsc_log_debug(log,"Execution of SmppWriter finished");  
  return 0;
}

}//smpp
}//transport
}//scag
