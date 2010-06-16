#ifndef __SMSC_ADMIN_HSDAEMON_CHINDWAITER_H__
#define __SMSC_ADMIN_HSDAEMON_CHINDWAITER_H__

#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/threads/Thread.hpp"
#include "Interconnect.h"
#include "SnmpTrap.hpp"
#include "util/debug.h"

namespace smsc{
namespace admin{
namespace hsdaemon{


class ChildShutdownWaiter;

typedef std::vector<ChildShutdownWaiter*> WAITERS;
///<summary>child shutdown waiter</summary>
class ChildShutdownWaiter : public smsc::core::threads::Thread
{
private:
  static WAITERS startedWaiters;
  static Mutex startedWaitersMutex;

private:
  smsc::logger::Logger *logger;
  bool isStopping;
  bool isStopped_;
  pid_t pid;
  const char * const serviceId;
public:
  ChildShutdownWaiter(const char * const serviceId)
    : logger(Logger::getInstance("smsc.admin.daemon.ChildShutdownWaiter")), isStopping(false), isStopped_(false), pid(-1), serviceId(cStringCopy(serviceId))
  {}
  ~ChildShutdownWaiter()
  {
    Stop();
    delete serviceId;
  }

  void Stop()
  {
    isStopping = true;
  }

  virtual int Execute()
  {
    Service* svc=0;
    try {
      {
        MutexGuard servicesGuard(DaemonCommandDispatcher::servicesListMutex);
        MutexGuard configGuard(DaemonCommandDispatcher::configManagerMutex);
        DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
        svc=DaemonCommandDispatcher::services[serviceId];
      }
      pid = svc->start();
      if(pid==0)
      {
        throw Exception("Service::start('%s')returned 0",serviceId);
      }
    } catch (AdminException& e) {
      smsc_log_error(logger, "Couldn't start service \"%s\", nested: %s", serviceId, e.what());
      return -1;
    } catch (std::exception& e) {
      smsc_log_error(logger, "Couldn't start service \"%s\", nested: %s", serviceId, e.what());
      return -1;
    } catch (...) {
      smsc_log_error(logger, "Couldn't start service \"%s\"", serviceId);
      return -1;
    }

    char msg[1024];
    sprintf(msg,"CLEARED SYSTEM %s Started (AlarmId=%s; severity=1)",serviceId,serviceId);
    SnmpTrap("SYSTEM",serviceId,1,msg);

    while (!isStopping)
    {
      __trace2__("ChildShutdownWaiter : waitpid \"%s\" %d", serviceId, pid);
      pid_t chldpid = waitpid(pid, 0, 0);
      __trace2__("ChildShutdownWaiter : waitpid \"%s\" %d finished", serviceId, pid);
      if (chldpid == -1)
      {
        switch (errno)
        {
        case ECHILD:
          chldpid=pid;
          __trace2__("ChildShutdownWaiter: process with pid=%d already finished",pid);
          break;
        case EINTR:
          smsc_log_debug(logger, "interrupted");
          break;
        case EINVAL:
          smsc_log_error(logger, "invalid arguments");
          break;
        default:
          smsc_log_error(logger, "unknown error");
          break;
        }
      }
      if (chldpid > 0)
      {
#ifdef SMSC_DEBUG
        __trace2__("CHILD %u is finished", chldpid);
#endif
        MutexGuard a(DaemonCommandDispatcher::servicesListMutex);
        svc=DaemonCommandDispatcher::services.get(serviceId);
        std::string cmd=svc->getServiceDir();
        cmd+='/';
        cmd+=Service::hostDown;
        cmd+=' ';
        cmd+=svc->getHost();
        __trace2__("try to exec '%s'",cmd.c_str());
        std::system(cmd.c_str());

        bool needRestart=false;
        bool restartOk=false;

        if(svc->getStatus()!=Service::stopping || svc->getSwitchover())
        {
          needRestart=true;
          if(svc->getType()==ServiceInfo::failover)
          {
            __trace2__("restarting service %s at another node", serviceId);
            try{
              svc->setSwitchover(false);
              restartOk=icon->remoteStartService(serviceId);
            }catch(std::exception& e)
            {
              __trace2__("failed to start remote service '%s':%s", serviceId,e.what());
            }
          }
        }
        if (const char * const serviceId = DaemonCommandDispatcher::services.markServiceAsStopped(chldpid))
        {
          MutexGuard lock(DaemonCommandDispatcher::configManagerMutex);
          DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
        }
        if(needRestart && !restartOk)
        {
          ChildShutdownWaiter::startService(svc->getId());
        }
        isStopping = true;
      }

      sleep(1);
    }

    sprintf(msg,"ACTIVE SYSTEM %s Stopped (AlarmId=%s; severity=5)",serviceId,serviceId);
    SnmpTrap("SYSTEM",serviceId,5,msg);

    isStopped_ = true;
    __trace2__("ChildShutdownWaiter : waiter \"%s\" %d FINISHED", serviceId, pid);
    return 0;
  }

  const char * const getServiceId() const {return serviceId;}
  const bool isStopped() const {return isStopped_;}

  static bool shutdownInProgress;

  static void startService(const char * const serviceId)
  {
    if(shutdownInProgress)
    {
      __trace2__("shutdown in progress, service '%s' not started",serviceId);
      return;
    }
    __trace2__("ChildShutdownWaiter : start service \"%s\"", serviceId);
    cleanStoppedWaiters();
    MutexGuard guard(startedWaitersMutex);
    __trace2__("ChildShutdownWaiter : start service \"%s\" continued", serviceId);
    ChildShutdownWaiter* newWaiter = new ChildShutdownWaiter(serviceId);
    startedWaiters.push_back(newWaiter);
    newWaiter->Start();
    __trace2__("ChildShutdownWaiter : start service \"%s\" finished", serviceId);
  }

  static void cleanStoppedWaiters()
  {
    struct __local__ {
      static void CheckRemove(WAITERS::value_type& val) {
        if ( val->isStopped() ) {
          __trace2__("ChildShutdownWaiter::cleanStoppedWaiters : delete waiter for \"%s\"", val->getServiceId());
          val->WaitFor();
          delete val;
          val = 0;
        }
      }
    };
    __trace__("ChildShutdownWaiter : clean stopped waiters");
    MutexGuard guard(startedWaitersMutex);
    std::for_each(startedWaiters.begin(), startedWaiters.end(), &__local__::CheckRemove);
    WAITERS::iterator new_end = std::remove(startedWaiters.begin(), startedWaiters.end(), (ChildShutdownWaiter*)0);
    if ( new_end != startedWaiters.end() ) startedWaiters.erase( new_end, startedWaiters.end() );
    __trace__("ChildShutdownWaiter : clean stopped waiters finished");
  }

  static void stopWaiters()
  {
    __trace__("ChildShutdownWaiter::stopWaiters");
    MutexGuard guard(startedWaitersMutex);
    for (WAITERS::iterator i = startedWaiters.begin(); i != startedWaiters.end(); i++) {
      (*i)->Stop();
      (*i)->WaitFor();
      delete *i;
    }
    startedWaiters.clear();
    __trace__("ChildShutdownWaiter::stopWaiters finished");
  }
};

}
}
}
#endif
