#ifndef __SMSC_PROFILER_PROFILER_HPP__
#define __SMSC_PROFILER_PROFILER_HPP__

#include "sms/sms.h"
#include "smeman/smeman.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "smpp/smpp.h"
#include "core/buffers/IntHash.hpp"

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

namespace ProfileReportOptions{
  const int ReportNone  =0;
  const int ReportFull  =1;
  const int ReportFinal =3;
}

namespace ProfileCharsetOptions{
  const int Default    =smsc::smpp::DataCoding::SMSC7BIT;
  const int Latin1     =smsc::smpp::DataCoding::LATIN1;
  const int Ucs2       =smsc::smpp::DataCoding::UCS2;
  const int Ucs2AndLat =smsc::smpp::DataCoding::UCS2 | smsc::smpp::DataCoding::LATIN1;
  const int UssdIn7Bit =0x80;
}

/*
  if((profile_dcs&UssdIn7Bit)==UssdIn7Bit)
  {
    if(sms.ussd_service_op)
    {
      profile_dcs=Default;
    }else
    {
      profile_dcs=profile_dcs&(~UssdIn7Bit);
    }
  }
*/

namespace ProfilerMatchType{
  static const int mtDefault=0;
  static const int mtMask   =1;
  static const int mtExact  =2;
}

namespace HideOption{
  static const int hoDisabled=0;
  static const int hoEnabled =1;
  static const int hoSubstitute=2;
}

inline const char* HideOptionToText(int ho)
{
  switch(ho)
  {
    case HideOption::hoDisabled:return "N";
    case HideOption::hoEnabled:return "Y";
    case HideOption::hoSubstitute:return "S";
  }
  return "U";
}

struct Profile{
  int codepage;
  int reportoptions;
  int hide;
  std::string locale;
  bool hideModifiable;

  std::string divert;
  bool divertActive;
  bool divertActiveAbsent;
  bool divertActiveBlocked;
  bool divertActiveBarred;
  bool divertActiveCapacity;
  bool divertModifiable;

  bool udhconcat;
  bool translit;

  Profile()
  {
    codepage=0;
    reportoptions=ProfileReportOptions::ReportFull;
    locale="";
    hide=0;
    hideModifiable=false;

    divert="";
    divertActive=false;
    divertModifiable=false;
    divertActiveAbsent=false;
    divertActiveBlocked=false;
    divertActiveBarred=false;
    divertActiveCapacity=false;
    udhconcat=true;
    translit=true;
  }

  Profile(const Profile& src)
  {
    *this=src;
  }

  Profile& operator=(const Profile& src)
  {
    codepage=src.codepage;
    reportoptions=src.reportoptions;
    locale=src.locale;
    hide=src.hide;
    hideModifiable=src.hideModifiable;

    divert=src.divert;
    divertModifiable=src.divertModifiable;
    divertActive=src.divertActive;
    divertActiveAbsent=src.divertActiveAbsent;
    divertActiveBlocked=src.divertActiveBlocked;
    divertActiveBarred=src.divertActiveBarred;
    divertActiveCapacity=src.divertActiveCapacity;

    udhconcat=src.udhconcat;
    translit=src.translit;
    return *this;
  }

  bool operator==(const Profile& src)
  {
    return codepage==src.codepage &&
           reportoptions==src.reportoptions &&
           locale==src.locale &&
           hide==src.hide &&
           hideModifiable==src.hideModifiable &&
           divert==src.divert &&
           divertActive==src.divertActive &&
           divertModifiable==src.divertModifiable &&
           divertActiveAbsent==src.divertActiveAbsent &&
           divertActiveBlocked==src.divertActiveBlocked &&
           divertActiveBarred==src.divertActiveBarred &&
           divertActiveCapacity==src.divertActiveCapacity &&
           udhconcat==src.udhconcat &&
           translit==src.translit
           ;
  }

  void assign(const Profile& src)
  {
    *this=src;
  }
};

class ProfilesTable;

const int pusUpdated=1;
const int pusInserted=2;
const int pusUnchanged=3;
const int pusError=4;

class ProfilerInterface{
public:
  virtual Profile& lookup(const Address& address)=0;
  virtual Profile& lookupEx(const Address& address,int& matchType,std::string& matchAddr)=0;
  virtual int update(const Address& address,const Profile& profile)=0;
  virtual int updatemask(const Address& address,const Profile& profile)=0;
  virtual void add(const Address& address,const Profile& profile)=0;
  virtual void remove(const Address& address)=0;

};//ProfilerInterface


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

  void dbUpdate(const Address& addr,const Profile& profile);
  void dbInsert(const Address& addr,const Profile& profile);

  void internal_update(int flag,const Address& addr,int value,const char* svalue=NULL);

};

}//profiler
}//smsc

#endif
