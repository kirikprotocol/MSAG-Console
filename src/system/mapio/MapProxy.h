#ifndef __SYSTEM_MAPIO_MAPPROXY_H__
#define __SYSTEM_MAPIO_MAPPROXY_H__

#include "smeman/smeproxy.h"
#include "logger/Logger.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/CyclicQueue.hpp"
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

extern void setMapProxyLimits(int timeout, int limit);

#define MAP_PROXY_QUEUE_LIMIT 512

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

//typedef smsc::core::buffers::Array<SmscCommand> MapIOQueue;
typedef smsc::core::buffers::CyclicQueue<SmscCommand> MapIOQueue;

class MapProxy:public SmeProxy{
public:
  MapProxy() : seq(0),smereg(0),managerMonitor(0) {
   time_logger = smsc::logger::Logger::getInstance("map.otime");
  }
  virtual ~MapProxy()
  {
    if(smereg)smereg->unregisterSmeProxy(this);
  }
  virtual void close(){}
  void notifyOutThread(){}
  void checkLogging();
  virtual void updateSmeInfo(const SmeInfo& smeInfo){
    setMapProxyLimits(smeInfo.timeout, smeInfo.proclimit);
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
  virtual bool getCommand(SmscCommand& cmd)
  {
    MutexGuard g(mutex);
    if(inqueue.Count()==0)return false;
    inqueue.Pop(cmd);
    return true;
  }

  virtual int getCommandEx(std::vector<SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    MutexGuard g(mutex);
    if(inqueue.Count()==0)
    {
      mx=0;
      return 0;
    }
    int cnt=inqueue.Count()<mx?inqueue.Count():mx;
    SmscCommand cmd;
    for(int i=0;i<cnt;i++)
    {
      inqueue.Pop(cmd);
      cmds.push_back(cmd);
      cmd.setProxy(prx);
    }
    mx=cnt;
    return inqueue.Count();
  }


  void putIncomingCommand(const SmscCommand& cmd)
  {
    {
      MutexGuard g(mutex);
      if( cmd->get_commandId() == SUBMIT && inqueue.Count()>=MAP_PROXY_QUEUE_LIMIT )
      {
        __mapproxy_trace__("putIncomingCommand: proxy queue limit exceded");
        throw ProxyQueueLimitException(inqueue.Count(),MAP_PROXY_QUEUE_LIMIT);
      }
      inqueue.Push(cmd);
    }
    if( managerMonitor ) managerMonitor->Signal();
  }

  SmscCommand getOutgoingCommand()
  {
    MutexGuard g(mutex);
    SmscCommand cmd;
    outqueue.Pop(cmd);
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
    MutexGuard g(seqMutex);
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
  mutable Mutex seqMutex;
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
