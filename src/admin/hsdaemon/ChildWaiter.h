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
    try {
      MutexGuard servicesGuard(DaemonCommandDispatcher::servicesListMutex);
      MutexGuard configGuard(DaemonCommandDispatcher::configManagerMutex);
      DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
      pid = DaemonCommandDispatcher::services[serviceId]->start();
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
      else if (chldpid > 0)
      {
#ifdef SMSC_DEBUG
        __trace2__("CHILD %u is finished", chldpid);
#endif
        MutexGuard a(DaemonCommandDispatcher::servicesListMutex);
        if(DaemonCommandDispatcher::services.get(serviceId)->getStatus()!=Service::stopping)
        {
          __trace2__("restarting service %s at another node", serviceId);
          try{
            icon->remoteStartService(serviceId);
          }catch(std::exception& e)
          {
            __trace2__("failed to start remote service '%s':%s", serviceId,e.what());
          }
        }
        if (const char * const serviceId = DaemonCommandDispatcher::services.markServiceAsStopped(chldpid))
        {
          MutexGuard lock(DaemonCommandDispatcher::configManagerMutex);
          DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
        }
        isStopping = true;
      }

      sleep(1);
    }
    isStopped_ = true;
    __trace2__("ChildShutdownWaiter : waiter \"%s\" %d FINISHED", serviceId, pid);
    return 0;
  }

  const char * const getServiceId() const {return serviceId;}
  const bool isStopped() const {return isStopped_;}

  static void startService(const char * const serviceId)
  {
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
