#ifndef __SMSC_SYSTEM_STATUSSME_HPP__
#define __SMSC_SYSTEM_STATUSSME_HPP__
#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "util/debug.h"
#include "system/smsc.hpp"

namespace smsc{
namespace system{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::profiler;
using smsc::system::Smsc;

class StatusSme:public SmeProxy,public ThreadedTask{
public:
  StatusSme(Smsc* smsc,const char* sysId):
      seq(1),
      managerMonitor(NULL),
      smsc(smsc),
      systemId(sysId)
  {
    prio=SmeProxyPriorityDefault;
  }

  ~StatusSme()
  {
    __trace__("try to unregister abonentinfo sme");
    try{
      smsc->unregisterSmeProxy(systemId);
    }catch(...)
    {
      __trace__("failed to unregister abonentinfo sme");
    }
  }

  const char* taskName(){return "StatusSme";}

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
    __trace__("StatusSme: init");
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

  void attachMonitor(ProxyMonitor* _mon)
  {
    managerMonitor=_mon;
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

  void setSources(const Address& hr,const Address& m)
  {
    hrSrc=hr;
    mSrc=m;
  }

  const char* getSystemId()const{return systemId.c_str();}

  string servType;
  int protId;

protected:
  mutable EventMonitor mon;
  smsc::core::buffers::Array<SmscCommand> outQueue;
  smsc::core::buffers::Array<SmscCommand> inQueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  Smsc *smsc;
  string systemId;
  SmeProxyPriority prio;
  Address hrSrc;
  Address mSrc;
};

}//system
}//smsc

#endif
