#ifndef __SMSC_PROFILER_PROFILER_HPP__
#define __SMSC_PROFILER_PROFILER_HPP__

#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadedTask.hpp"

namespace smsc{
namespace db{
class DataSource;
};
namespace profiler{

using namespace smsc::sms;
using namespace smsc::smeman;
using smsc::core::threads::ThreadedTask;
using namespace smsc::core::synchronization;

namespace ProfileReportOptions{
  static const int ReportNone =0;
  static const int ReportFull =1;
};

namespace ProfileCharsetOptions{
  static const int Default    =0;
  static const int Ucs2       =1;
};

struct Profile{
  int codepage;
  int reportoptions;
  bool operator==(const Profile& src)
  {
    return codepage==src.codepage &&
           reportoptions==src.reportoptions;
  }
  void assign(const Profile& src)
  {
    codepage=src.codepage;
    reportoptions=src.reportoptions;
  }
};

class ProfilesTable;

class ProfilerInterface{
public:
  virtual Profile& lookup(const Address& address)=0;
  virtual void update(const Address& address,const Profile& profile)=0;
  virtual void add(const Address& address,const Profile& profile)=0;
};//ProfilerInterface


class Profiler:public ProfilerInterface,public SmeProxy, public ThreadedTask{
public:
  Profiler(const Profile& pr);
  virtual ~Profiler();

  int Execute();
  const char* taskName(){return "Profiler";}

  /* from ProfilerInterface */
  Profile& lookup(const Address& address);

  void update(const Address& address,const Profile& profile);

  void add(const Address& address,const Profile& profile);

  /* from SmppProxy */

  void close(){}

  // smsc call this methid to enqueue commands
  void putCommand(const SmscCommand& cmd)
  {
    MutexGuard g(mon);
    mon.notify();
    outQueue.Push(cmd);
  }

  //Profiler will never generate a command
  SmscCommand getCommand(){SmscCommand cmd;return cmd;};

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

  //Profiler will never generate a command
  bool hasInput()const
  {
    return false;
  }

  void attachMonitor(ProxyMonitor* mon)
  {
    managerMonitor=mon;
  }
  bool attached()
  {
    return managerMonitor!=NULL;
  }

  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mon);
    return seq++;
  }

  void loadFromDB();

protected:
  mutable EventMonitor mon;
  smsc::core::buffers::Array<SmscCommand> outQueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  ProfilesTable *profiles;

  smsc::db::DataSource *ds;

  void dbUpdate(const Address& addr,const Profile& profile);
  void dbInsert(const Address& addr,const Profile& profile);

  void internal_update(int flag,const Address& addr,int value);

};

};//profiler
};//smsc

#endif
