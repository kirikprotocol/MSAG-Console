#ifndef __SMSC_SYSTEM_SMSCSME_HPP__
#define __SMSC_SYSTEM_SMSCSME_HPP__
#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "profiler/profiler.hpp"
#include "util/debug.h"
#include "core/buffers/CyclicQueue.hpp"

namespace smsc{
namespace system{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::profiler;

class SmscSme:public SmeProxy,public ThreadedTask{
public:
  SmscSme(const char* sysId,SmeRegistrar* smeman):
      seq(1),
      managerMonitor(NULL),
      systemId(sysId),
      smeman(smeman)
  {
    prio=SmeProxyPriorityDefault;
  }

  ~SmscSme()
  {
    __trace__("try to unregister abonentinfo sme");
    try{
      smeman->unregisterSmeProxy(systemId);
    }catch(...)
    {
      __trace__("failed to unregister abonentinfo sme");
    }
  }

  const char* taskName(){return "SmscSme";}

  virtual int Execute();

  void putCommand(const SmscCommand& cmd)
  {
    MutexGuard g(mon);
    mon.notify();
    outQueue.Push(cmd);
  }

  SmscCommand getCommand()
  {
    MutexGuard g(mon);
    SmscCommand cmd;
    if(inQueue.Count()==0)return cmd;
    inQueue.Pop(cmd);
    return cmd;
  };

  void putIncomingCommand(const SmscCommand& cmd)
  {
    mon.Lock();
    inQueue.Push(cmd);
    mon.Unlock();
    managerMonitor->Signal();
  }


  //Used by profiler to retrieve commands sent by smsc
  SmscCommand getOutgoingCommand()
  {
    MutexGuard g(mon);
    SmscCommand cmd;
    outQueue.Pop(cmd);
    return cmd;
  }

  bool hasOutput()
  {
    MutexGuard g(mon);
    return outQueue.Count()!=0;
  }

  void waitFor()
  {
    mon.Lock();
    if(!outQueue.Count())
    {
      mon.wait();
    }
    mon.Unlock();
  }

  SmeProxyState getState()const
  {
    return state;
  }

  void init()
  {
    __trace__("SmscSme: init");
    managerMonitor=NULL;
    state=VALID;
    seq=1;
  }

  SmeProxyPriority getPriority()const
  {
    return prio;
  }
  void setPriority(SmeProxyPriority newprio)
  {
    prio=newprio;
  }

  bool hasInput()const
  {
    MutexGuard g(mon);
    return inQueue.Count()!=0;
  }

  void attachMonitor(ProxyMonitor* __mon)
  {
    managerMonitor=__mon;
  }
  bool attached()
  {
    return managerMonitor!=NULL;
  }

  void close(){}

  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mon);
    return seq++;
  }

  const char* getSystemId()const{return systemId.c_str();}

  string servType;
  int protId;

protected:
  mutable EventMonitor mon;
  smsc::core::buffers::CyclicQueue<SmscCommand> outQueue;
  smsc::core::buffers::CyclicQueue<SmscCommand> inQueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  string systemId;
  SmeRegistrar *smeman;
  SmeProxyPriority prio;
};

}//system
}//smsc

#endif
