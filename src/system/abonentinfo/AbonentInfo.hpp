#ifndef __SMSC_SYSTEM_ABONENTINFO_HPP__
#define __SMSC_SYSTEM_ABONENTINFO_HPP__
#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "profiler/profiler.hpp"
#include "util/debug.h"

namespace smsc{
namespace system{
namespace abonentinfo{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::profiler;

class AbonentInfoSme:public SmeProxy,public ThreadedTask{
public:
  AbonentInfoSme(ProfilerInterface* profiler,
    SmeRegistrar* smeman,const char* sysId):profiler(profiler),smeman(smeman),systemId(sysId)
  {
  }

  ~AbonentInfoSme()
  {
    __trace__("try to unregister abonentinfo sme");
    try{
      smeman->unregisterSmeProxy(systemId);
    }catch(...)
    {
      __trace__("failed to unregister abonentinfo sme");
    }
  }

  const char* taskName(){return "AbonentInfo";}

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
    inQueue.Shift(cmd);
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
    outQueue.Shift(cmd);
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
    mon.wait();
    mon.Unlock();
  }

  SmeProxyState getState()const
  {
    return state;
  }

  void init()
  {
    managerMonitor=NULL;
    state=VALID;
  }

  SmeProxyPriority getPriority()const
  {
    return SmeProxyPriorityDefault;
  }

  bool hasInput()const
  {
    MutexGuard g(mon);
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
    MutexGuard g(mon);
    return seq++;
  }
protected:
  mutable EventMonitor mon;
  smsc::core::buffers::Array<SmscCommand> outQueue;
  smsc::core::buffers::Array<SmscCommand> inQueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  ProfilerInterface* profiler;
  SmeRegistrar *smeman;
  string systemId;
};

};//abonentinfo
};//system
};//smsc

#endif
