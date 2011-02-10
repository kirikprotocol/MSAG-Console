#ifndef __SMSC_SYSTEM_ABONENTINFO_HPP__
#define __SMSC_SYSTEM_ABONENTINFO_HPP__
#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "profiler/profiler.hpp"
#include "util/debug.h"
#include "system/smsc.hpp"

namespace smsc{
namespace system{
namespace abonentinfo{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::profiler;
using smsc::system::Smsc;

class AbonentInfoSme:public SmeProxy,public ThreadedTask{
public:
  AbonentInfoSme(Smsc* smsc,const char* sysId):
      seq(1),
      managerMonitor(NULL),
      smsc(smsc),
      systemId(sysId)
  {
    prio=SmeProxyPriorityDefault;
  }

  virtual ~AbonentInfoSme()
  {
    __trace__("try to unregister abonentinfo sme");
    try{
      smsc->unregisterSmeProxy(this);
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

  bool getCommand(SmscCommand& cmd)
  {
    MutexGuard g(mon);
    if(inQueue.Count()==0)return false;
    inQueue.Shift(cmd);
    return true;
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
    __trace__("AbonentInfo: init");
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

}//abonentinfo
}//system
}//smsc

#endif
