#ifndef __SYSTEM_SMPPIO_SMPPPROXY_HPP__
#define __SYSTEM_SMPPIO_SMPPPROXY_HPP__

#include "smeman/smeproxy.h"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "system/smppio/SmppSocket.hpp"
#include "smeman/smsccmd.h"
#include <string>

namespace smsc{
namespace system{
namespace smppio{

#define SMPP_PROXY_QUEUE_LIMIT 64

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

class SmppSocket;

class SmppProxy:public SmeProxy{
public:
  SmppProxy(SmppSocket* sock):smppSocket(sock)
  {
    smppSocket->assignProxy(this);
    seq=1;
    managerMonitor=NULL;
  }
  virtual ~SmppProxy(){}
  virtual void close()
  {

  }
  virtual void putCommand(const SmscCommand& cmd)
  {
    trace("put command:enter");
    {
      MutexGuard g(mutex);
      if(outqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
      {
        throw ProxyQueueLimitException();
      }
      trace2("put command:%p",*((void**)&cmd));
      outqueue.Push(cmd);
    }
    smppSocket->notifyOutThread();
  }
  virtual SmscCommand getCommand()
  {
    MutexGuard g(mutex);
    SmscCommand cmd;
    inqueue.Shift(cmd);
    trace2("get command:%p",*((void**)&cmd));
    return cmd;
  }

  void putIncomingCommand(const SmscCommand& cmd)
  {
    mutex.Lock();
    if(inqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
    {
      mutex.Unlock();
      throw ProxyQueueLimitException();
    }
    inqueue.Push(cmd);
    mutex.Unlock();
    managerMonitor->Signal();
  }
  SmscCommand getOutgoingCommand()
  {
    mutex.Lock();
    SmscCommand cmd;
    outqueue.Shift(cmd);
    mutex.Unlock();
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
    managerMonitor=NULL;
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
    return seq++;
  }

  void setId(const std::string newid)
  {
    id=newid;
  }

  std::string getId(){return id;}

protected:
  mutable Mutex mutex;
  std::string id;
  smsc::core::buffers::Array<SmscCommand> inqueue,outqueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  SmppSocket *smppSocket;
};

};//smppio
};//system
};//smsc


#endif
