#ifndef __SMSC_PROFILER_PROFILER_HPP__
#define __SMSC_PROFILER_PROFILER_HPP__

#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "smpp/smpp.h"
#include "core/buffers/IntHash.hpp"
#include "profiler-types.hpp"
#include "profile-notifier-iface.hpp"

namespace smsc{
namespace db{
class DataSource;
}
namespace profiler{

using namespace smsc::sms;
using namespace smsc::smeman;
using smsc::core::threads::ThreadedTask;
using namespace smsc::core::synchronization;

using smsc::core::buffers::IntHash;


class ProfilesTable;

class Profiler:public ProfilerInterface,public SmeProxy, public ThreadedTask{
public:
  Profiler(const Profile& pr,SmeRegistrar* smeman,const char* sysId);
  virtual ~Profiler();

  int Execute();
  const char* taskName(){return "Profiler";}

  /* from ProfilerInterface */
  Profile& lookup(const Address& address);
  Profile& lookupEx(const Address& address,int& matchType,std::string& matchAddr);

  int update(const Address& address,const Profile& profile);
  int updatemask(const Address& address,const Profile& profile);

  void add(const Address& address,const Profile& profile);

  void remove(const Address& address);

  /* from SmppProxy */

  void close(){}

  // smsc call this methid to enqueue commands
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
    managerMonitor=NULL;
    state=VALID;
  }

  SmeProxyPriority getPriority()const
  {
    return prio;
  }
  void setPriority(SmeProxyPriority newprio)
  {
    prio=newprio;
  }

  //Profiler will never generate a command
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

  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mon);
    return seq++;
  }

  void loadFromDB(smsc::db::DataSource *datasrc);

  string serviceType;
  int protocolId;

  const char* getSystemId()const{return systemId.c_str();}

  void addToUssdCmdMap(int cmd,const string& txt)
  {
    ussdCmdMap.Insert(cmd,txt);
  }

  void setNotifier(ProfileNotifierInterface* pnotifier)
  {
    notifier=pnotifier;
  }

protected:
  mutable EventMonitor mon;
  smsc::core::buffers::Array<SmscCommand> outQueue;
  smsc::core::buffers::Array<SmscCommand> inQueue;
  int seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  ProfilesTable *profiles;
  SmeRegistrar *smeman;
  Mutex mtx;

  smsc::logger::Logger *log;

  IntHash<string> ussdCmdMap;

  string systemId;
  SmeProxyPriority prio;

  smsc::db::DataSource *ds;
  ProfileNotifierInterface* notifier;

  void dbUpdate(const Address& addr,const Profile& profile);
  void dbInsert(const Address& addr,const Profile& profile);

  void internal_update(int flag,const Address& addr,int value,const char* svalue=NULL);

};

}//profiler
}//smsc

#endif
