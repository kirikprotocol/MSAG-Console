#ifndef __SYSTEM_SMPPIO_SMPPPROXY_HPP__
#define __SYSTEM_SMPPIO_SMPPPROXY_HPP__

#include "smeman/smeproxy.h"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "system/smppio/SmppSocket.hpp"
#include "smeman/smsccmd.h"

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
  }
  virtual void close()
  {

  }
  virtual void putCommand(const SmscCommand& cmd)
  {
    mutex.Lock();
    if(outqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
    {
      mutex.Unlock();
      throw ProxyQueueLimitException();
    }
    outqueue.Push(cmd);
    smppSocket->notifyOutThread();
    mutex.Unlock();
  }
  virtual SmscCommand getCommand()
  {
    mutex.Lock();
    SmscCommand cmd;
    inqueue.Shift(cmd);
    mutex.Unlock();
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
    managerMonitor->Signal();
    mutex.Unlock();
  }
  SmscCommand getOutgoingCommand()
  {
    mutex.Lock();
    SmscCommand cmd;
    outqueue.Shift(cmd);
    mutex.Unlock();
    return cmd;
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
  virtual SmeProxyPriority getPriority()const{}
  bool hasInput()const
  {
    mutex.Lock();
    int retval=inqueue.Count();
    mutex.Unlock();
    return retval;
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

protected:
  mutable Mutex mutex;
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
