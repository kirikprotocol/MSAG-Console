#include "SmppWriter2.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace transport{
namespace smpp{

using util::HRTimer;

int SmppWriter::Execute()
{
  smsc_log_debug(log,"Starting SmppWriter");
  net::Multiplexer::SockArray ready,error;
    unsigned pass = 0;
  while(!isStopping)
  {
      bool dotiming = ++pass % 100;
      HRTimer hrt;
      if (dotiming) hrt.mark();
      hrtime_t timeprep, timepoll, timewrite;
      unsigned haveData = 0;
      {
          MutexGuard mg(mon);
          while(sockets.Count()==0)
          {
              mon.wait(2000);
              if(isStopping)break;
          }
          if(isStopping)break;

          mul.clear();
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
                  ++haveData;
              }
          }
          if(!haveData)
          {
              mon.wait(500);
              deleteDisconnected();
              continue;
          }
      }

      if (dotiming) timeprep = hrt.get();

      if (mul.canWrite(ready,error,500) )
      {
          if (dotiming) timepoll = hrt.get();
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
          if (dotiming) timewrite = hrt.get();
          MutexGuard mg(mon);
          deleteDisconnected();
      }

      if (dotiming) smsc_log_info( log, "timing (us): socks=%d prep=%lld poll=%lld write=%lld",
                                   haveData, timeprep/1000, timepoll/1000, timewrite/1000 );

  } // while ! stopping
  deleteDisconnected();
  smsc_log_debug(log,"Execution of SmppWriter finished");  
  return 0;
}

}//smpp
}//transport
}//scag
