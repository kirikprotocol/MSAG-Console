#ifndef __SMSC_SYSTEM_MAPSYM_HPP__
#define __SMSC_SYSTEM_MAPSYM_HPP__
#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Event.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "util/debug.h"

#include <unistd.h>

namespace smsc{
namespace system{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;

class MapSymProxy:public SmeProxy{
public:
  MapSymProxy():
      seq(1),
      managerMonitor(NULL)
  {
  }

  ~MapSymProxy()
  {
  }

  void putCommand(const SmscCommand& cmd)
  {
    MutexGuard g(outmon);
    outmon.notify();
    outQueue.Push(cmd);
  }

  SmscCommand getCommand()
  {
    MutexGuard g(mutexin);
    SmscCommand cmd;
    inQueue.Shift(cmd);
    return cmd;
  };

  void putIncomingCommand(const SmscCommand& cmd)
  {
    {
      MutexGuard g(mutexin);
      inQueue.Push(cmd);
    }
    managerMonitor->Signal();
  }


  //Used by profiler to retrieve commands sent by smsc
  SmscCommand getOutgoingCommand()
  {
    MutexGuard g(outmon);
    SmscCommand cmd;
    outQueue.Shift(cmd);
    return cmd;
  }

  bool hasOutput()
  {
    MutexGuard g(outmon);
    return outQueue.Count()!=0;
  }

  void waitFor()
  {
    outmon.Lock();
    if(!outQueue.Count())
    {
      outmon.wait();
    }
    outmon.Unlock();
  }

  SmeProxyState getState()const
  {
    return state;
  }

  void init()
  {
    managerMonitor=NULL;
    state=VALID;
    seq=1;
  }

  SmeProxyPriority getPriority()const
  {
    return SmeProxyPriorityDefault;
  }

  bool hasInput()const
  {
    MutexGuard g(mutexin);
    return inQueue.Count()!=0;
  }

  void attachMonitor(ProxyMonitor* mon)
  {
    managerMonitor=mon;
  }
  bool attached()
  {
    return managerMonitor!=NULL;
  }

  void close(){}

  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mutex);
    return seq++;
  }

  const char* getSystemId()const{return "MAPSYM";}

  string servType;
  int protId;

protected:
  mutable Mutex mutex,mutexin;
  mutable EventMonitor outmon;
  smsc::core::buffers::Array<SmscCommand> outQueue;
  smsc::core::buffers::Array<SmscCommand> inQueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
};

class MapSymInputTask:public ThreadedTask{
protected:
  MapSymProxy* proxy;
public:
  MapSymInputTask(MapSymProxy* proxy):proxy(proxy){}
  const char* taskName(){return "MapSymInput";}
  int Execute()
  {
    SMS sms;
    sleep(5);
    sms.setOriginatingAddress("666");
    sms.setDestinationAddress("666");
    char msc[]="123";
    char imsi[]="123";
    sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    sms.setValidTime(0);
    sms.setDeliveryReport(0);
    sms.setArchivationRequested(false);
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    sms.setMessageReference(0);
    sms.setIntProperty(Tag::SMPP_DATA_CODING,0);

    srand(time(NULL));

    const int MINLEN=5;
    const int MAXLEN=100;
    char msg[MAXLEN+1];
    Event ev;

    const char *letters="abcdefghijklmnopqrstuvwxyz0123456789";

    while(!isStopping)
    {
      for(int j=0;j<50;j++)
      {
        int len=(int)(((double)rand())*(MAXLEN-MINLEN)/RAND_MAX+MINLEN);
        for(int i=0;i<len;i++)
        {
          msg[i]=letters[rand()%strlen(letters)];
        }
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msg,len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,len);
        try{
          proxy->putIncomingCommand(
            SmscCommand::makeSumbmitSm(sms,proxy->getNextSequenceNumber())
          );
        }catch(...)
        {
        }
      }
      ev.Wait(10);
      thr_yield();
    }
    return 0;
  }
};//MapSymInputTask

class MapSymOutputTask:public ThreadedTask{
protected:
  MapSymProxy* proxy;
public:
  MapSymOutputTask(MapSymProxy* proxy):proxy(proxy){}
  const char* taskName(){return "MapSymOutput";}
  int Execute()
  {
    while(!isStopping)
    {
      proxy->waitFor();
      if(!proxy->hasOutput() && isStopping)break;
      SmscCommand cmd=proxy->getOutgoingCommand();
      if(cmd->get_commandId()==DELIVERY)
      {
        __trace__("MAPSYM: delivery received");
        try{
          proxy->putIncomingCommand(
            SmscCommand::makeDeliverySmResp("0",cmd->get_dialogId(),0)
          );
        }catch(...)
        {
        }
      }
    }
    return 0;
  }
};
};//system
};//smsc

#endif
