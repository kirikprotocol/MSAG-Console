#ifndef __SYSTEM_MAPIO_MAPPROXY_H__
#define __SYSTEM_MAPIO_MAPPROXY_H__

#include "smeman/smeproxy.h"
#include "logger/Logger.h"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "smeman/smsccmd.h"
#include "smeman/smereg.h"
#include "core/buffers/IntHash.hpp"
#include <string>

//#if defined USE_MAP
extern void MAPIO_PutCommand(const smsc::smeman::SmscCommand& cmd );
//#endif

namespace smsc{
namespace logger{
extern smsc::logger::Logger* _mapproxy_cat;
}
}
#define __mapproxy_trace2__(format,...) __log2__(smsc::logger::_mapproxy_cat,smsc::logger::Logger::LEVEL_DEBUG,format,__VA_ARGS__)
#define __mapproxy_trace__(text) __debug__(smsc::logger::_mapproxy_cat,text)



namespace smsc{
namespace system{
namespace mapio{

#define MAP_PROXY_QUEUE_LIMIT 16*1024

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

typedef smsc::core::buffers::Array<SmscCommand> MapIOQueue;

class MapProxy:public SmeProxy{
public:
  MapProxy() : seq(0),smereg(0) {
   time_logger = smsc::logger::Logger::getInstance("map.otime");
  }
  virtual ~MapProxy()
  {
    if(smereg)smereg->unregisterSmeProxy("MAP_PROXY");
  }
  virtual void close(){}
  void notifyOutThread(){}
  void checkLogging();
  virtual void updateSmeInfo(const SmeInfo& smeInfo){
    MapDialogContainer::getInstance()->setPerformanceLimits(smeInfo.timeout, smeInfo.proclimit);
  }
  virtual void putCommand(const SmscCommand& cmd)
  {
    struct timeval utime, curtime;
    if( time_logger->isDebugEnabled() ) gettimeofday( &utime, 0 );
    ::MAPIO_PutCommand(cmd);
    if( time_logger->isDebugEnabled() ) {
      long usecs;
      gettimeofday( &curtime, 0 );
      usecs = curtime.tv_usec < utime.tv_usec?(1000000+curtime.tv_usec)-utime.tv_usec:curtime.tv_usec-utime.tv_usec;
      smsc_log_debug(time_logger, "cmdid=%d s=%ld us=%ld", cmd->get_commandId(), curtime.tv_sec-utime.tv_sec, usecs );
    }
  }
  virtual SmscCommand getCommand()
  {
    MutexGuard g(mutex);
    SmscCommand cmd;
    inqueue.Shift(cmd);
    __mapproxy_trace2__("get command:%p",*((void**)&cmd));
    return cmd;
  }

  void putIncomingCommand(const SmscCommand& cmd)
  {
    __mapproxy_trace__("putIncomingCommand");
    {
      MutexGuard g(mutex);
      if(inqueue.Count()==MAP_PROXY_QUEUE_LIMIT)
      {
        __mapproxy_trace__("putIncomingCommand: proxy queue limit exceded");
        throw ProxyQueueLimitException();
      }
      inqueue.Push(cmd);
    }
    managerMonitor->Signal();
  }

  SmscCommand getOutgoingCommand()
  {
    __mapproxy_trace__("getOutgoingCommand");
    MutexGuard g(mutex);
    SmscCommand cmd;
    outqueue.Shift(cmd);
    return cmd;
  }

  bool hasOutput()
  {
    MutexGuard g(mutex);
    return outqueue.Count()!=0;
  }

  virtual SmeProxyState getState()const
  {
    return state;
  }

  void init()
  {
    managerMonitor=0;
    state=VALID;
  }

  virtual SmeProxyPriority getPriority()const{return SmeProxyPriorityDefault;}

  bool hasInput()const
  {
    MutexGuard g(mutex);
    return inqueue.Count()!=0;
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
    if (seq <  0x40000) seq = 0x40000;
    return seq++;
  }

  void setId(const std::string newid)
  {
    id=newid;
  }

  virtual unsigned long getPreferredTimeout() { return 45; }

  std::string getId(){return id;}

  const char* getSystemId()const{return id.c_str();}

  void assignSmeRegistrar(SmeRegistrar* r)
  {
    smereg=r;
  }

protected:
  mutable Mutex mutex;
  std::string id;
  MapIOQueue inqueue,outqueue;
  uint32_t seq;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  smsc::logger::Logger* time_logger;
  SmeRegistrar *smereg;
};

}//mappio
}//system
}//smsc


#endif
