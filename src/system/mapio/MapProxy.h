
#ifndef __SYSTEM_MAPIO_MAPPROXY_H__
#define __SYSTEM_MAPIO_MAPPROXY_H__

#include "smeman/smeproxy.h"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "smeman/smsccmd.h"
#include <string>

namespace smsc{
namespace system{
namespace mapio{

#define MAP_PROXY_QUEUE_LIMIT 64

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

typedef smsc::core::buffers::Array<SmscCommand> MapIOQueue;

class MapProxy:public SmeProxy{
public:
  MapProxy() : seq(0) {}
  virtual ~MapProxy(){}
  virtual void close(){}
  void notifyOutThread(){}
  virtual void putCommand(const SmscCommand& cmd);
  //{
  //  __trace2__("MAPPROXY::putCommand");
  //  {
  //    MutexGuard g(mutex);
  //    /*if(outqueue.Count()==MAP_PROXY_QUEUE_LIMIT)
  //    {
  //      throw ProxyQueueLimitException();
  //    }
  //    __trace2__("MAPPROXY::put command:%p",*((void**)&cmd));
  //    outqueue.Push(cmd);*/
  //
  //  }
  //  //notifyOutThread();
  //}
  virtual SmscCommand getCommand()
  {
    MutexGuard g(mutex);
    SmscCommand cmd;
    inqueue.Shift(cmd);
    __trace2__("MAPPROXY::get command:%p",*((void**)&cmd));
    return cmd;
  }

  void putIncomingCommand(const SmscCommand& cmd)
  {
    __trace2__("MAPPROXY::putIncomingCommand");
    {
      MutexGuard(mutex);
      __trace2__("MAPPROXY::putIncomingCommand: locked");
      if(inqueue.Count()==MAP_PROXY_QUEUE_LIMIT)
      {
        throw ProxyQueueLimitException();
      }
      inqueue.Push(cmd);
    }
    __trace2__("MAPPROXY::putIncomingCommand: signal");
    managerMonitor->Signal();
    __trace2__("MAPPROXY::putIncomingCommand OK");
  }

  SmscCommand getOutgoingCommand()
  {
    __trace2__("MAPPROXY::getOutgoingCommand");
    MutexGuard g(mutex);
    SmscCommand cmd;
    outqueue.Shift(cmd);
    __trace2__("MAPPROXY::getOutgoingCommand OK");
    return cmd;
  }

  bool hasOutput()
  {
    MutexGuard g(mutex);
    return outqueue.Count()!=0;
  }

  virtual SmeProxyState getState()const
  {
    return state;
  }
  
  void init()
  {
    managerMonitor=0;
    state=VALID;
  }
  
  virtual SmeProxyPriority getPriority()const{return SmeProxyPriorityDefault;}
  
  bool hasInput()const
  {
    MutexGuard g(mutex);
    return inqueue.Count()!=0;
  }
  
  virtual void attachMonitor(ProxyMonitor* mon)
  {
    managerMonitor=mon;
  }
  
  virtual bool attached()
  {
    return managerMonitor!=NULL;
  }

  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mutex);
    __trace2__("MAP::Proxy:getNextSequenceNumber next number 0x%x",seq);
    if (seq <  0x10000) seq = 0x10000; 
    return seq++;
  }

  void setId(const std::string newid)
  {
    id=newid;
  }

  virtual unsigned long getPreferredTimeout() { return 45; }

  std::string getId(){return id;}

protected:
  mutable Mutex mutex;
  std::string id;
  MapIOQueue inqueue,outqueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
};

};//mappio
};//system
};//smsc


#endif
