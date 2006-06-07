#ifndef __SMSC_SYSTEM_COMMON_TIMEZONEMAN_HPP_
#define __SMSC_SYSTEM_COMMON_TIMEZONEMAN_HPP_

#include <string>
#include <map>
#include "core/synchronization/Mutex.hpp"
#include "sms/sms.h"
#include "util/Exception.hpp"

namespace smsc{
namespace system{
namespace common{

namespace sync=smsc::core::synchronization;

class TimeZoneManager{
public:
  TimeZoneManager()
  {
    log=smsc::logger::Logger::getInstance("tzman");
  }
  static void Init(const char* tzFileName,const char* routeFileName)
  {
    instance=new TimeZoneManager;
    instance->cfgTzFileName=tzFileName;
    instance->cfgRouteFileName=routeFileName;
    instance->Reload();
    smsc_log_info(instance->log,"init %s, %s",tzFileName,routeFileName);
  }

  static TimeZoneManager& getInstance()
  {
    if(!instance)throw smsc::util::Exception("TimeZoneManager wasn't initiated");
    return *instance;
  }

  void Reload();

  int getTimeZone(const smsc::sms::Address& addr)const
  {
    sync::MutexGuard mg(mtx);
    OffsetMap::const_iterator it=offsetMap.find(addr);
    if(it==offsetMap.end())
    {
      smsc_log_debug(log,"defTz=%d for addr=%s",defaultOffset,addr.toString().c_str());
      return defaultOffset;
    }
    smsc_log_debug(log,"tz=%d for addr=%s",it->second,addr.toString().c_str());
    return it->second;
  }

  static void Shutdown()
  {
    delete instance;
    instance=0;
  }

protected:

  static TimeZoneManager* instance;

  struct MaskComparator{
    bool operator()(const smsc::sms::Address& addra,const smsc::sms::Address& addrb)const
    {
      if(addra.type<addrb.type)return true;
      if(addra.type>addrb.type)return false;
      if(addra.plan<addrb.plan)return true;
      if(addra.plan>addrb.plan)return false;
      const char* a=addra.value;
      const char* b=addrb.value;
      while(*a && *b)
      {
        if(*a=='?' || *b=='?')return addra.length<addrb.length;//strlen(a)<strlen(b);
        if(*a<*b)return true;
        a++;
        b++;
      }
      return !*a && *b;
    }
  };


  std::string cfgTzFileName;
  std::string cfgRouteFileName;
  mutable sync::Mutex mtx;
  typedef std::map<smsc::sms::Address,int,MaskComparator> OffsetMap;
  OffsetMap offsetMap;

  int defaultOffset;

  smsc::logger::Logger* log;

};

}//common
}//system
}//smsc

#endif
