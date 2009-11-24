#ifndef __SMSC_SYSTEM_COMMON_TIMEZONEMAN_HPP_
#define __SMSC_SYSTEM_COMMON_TIMEZONEMAN_HPP_

#include <string>

#include "core/buffers/DigitalTree.hpp"
#include "core/synchronization/Mutex.hpp"
#include "sms/sms.h"
#include "util/Exception.hpp"


namespace smsc{
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
    smsc_log_info(instance->log,"Init: %s, %s",tzFileName,routeFileName);
    instance->Reload();
  }

  static TimeZoneManager& getInstance()
  {
    if(!instance)throw smsc::util::Exception("TimeZoneManager wasn't initiated");
    return *instance;
  }

  void Reload();

  int getTimeZone(const smsc::sms::Address& addr)const
  {
    char buf[64];
    sprintf(buf,"%d%d%s",addr.type%10,addr.plan%10,addr.value);
    sync::MutexGuard mg(mtx);
    int rv;
    if(!offsetMap.Find(buf,rv))
    {
      smsc_log_debug(log,"defTz=%d for addr=%s",defaultOffset,addr.toString().c_str());
      return defaultOffset;
    }
    smsc_log_debug(log,"tz=%d for addr=%s",rv,addr.toString().c_str());
    return rv;
  }

  static void Shutdown()
  {
    delete instance;
    instance=0;
  }

protected:

  static TimeZoneManager* instance;


  std::string cfgTzFileName;
  std::string cfgRouteFileName;
  mutable sync::Mutex mtx;
  smsc::core::buffers::DigitalTree<int> offsetMap;

  int defaultOffset;

  smsc::logger::Logger* log;

  std::string AddrToString(const smsc::sms::Address& addr)
  {
    char buf[64];
    sprintf(buf,"%d%d%s",addr.type%10,addr.plan%10,addr.value);
    return buf;
  }

};

}//common
}//smsc

#endif
