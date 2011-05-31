#include "Service.h"

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <system/smscsignalhandlers.h>
#include <sys/resource.h>
#include "Interconnect.h"
#include "DaemonCommandDispatcher.h"
#include "util/sleep.h"

namespace smsc {
namespace admin {
namespace hsdaemon {

const char * const Service::service_exe = "bin/service";
const char * Service::hostUp = "bin/hostup";
const char * Service::hostDown = "bin/hostdown";

bool checkExec(const char* binFile)
{
  struct stat st;
  if(stat(binFile,&st)!=0)return false;
  return (st.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH))!=0;
}

pid_t Service::start()
  throw (AdminException)
{
  __trace2__("Service::start(%s)",info.id.c_str());

  if(info.id.length()==0)throw AdminException("Service with empty name detected!");
  try{
    run_status st=icon->remoteGetServiceStatus(info.id.c_str());
    __trace2__("remote service status=%d",st);
    switch (st)
    {
    case starting:
      throw AdminException("Service already starting at another node");
    case running:
      throw AdminException("Service already running at another node");
    case stopping:
      throw AdminException("Service stopping at another node");
    }
  }catch(AdminException& e)
  {
    throw;
  }
  catch(std::exception& e)
  {
    __trace2__("Failed to get remote status of service '%s':%s",info.id.c_str(),e.what());
  }

  switch (status)
  {
  case starting:
    throw AdminException("Service already starting");
  case running:
    throw AdminException("Service already running");
  case stopping:
    throw AdminException("Service stopping");
  case stopped:
    {
      status=starting;
      struct StatusGuard{
        run_status& st;
        bool released;
        StatusGuard(run_status& st):st(st),released(false){}
        void release()
        {
          released=true;
        }
        ~StatusGuard()
        {
          if(!released)st=stopped;
        }
      }stguard(status);
      autoDelay();
      time_t now=time(NULL);
      if(now-lastStart<DaemonCommandDispatcher::stayAliveTimeout)
      {
        smsc_log_info(logger,"Delay service start after crash restart for %d seconds",DaemonCommandDispatcher::retryTimeout*(1+restartRetryCount));
        sleepEvent.Wait(1000*DaemonCommandDispatcher::retryTimeout*(1+restartRetryCount));
        restartRetryCount++;
      }else
      {
        restartRetryCount=0;
      }
      if(status==stopping)return 0;
      lastStart=time(NULL);

      if(!checkExec((serviceDir+'/'+service_exe).c_str()))
      {
        throw AdminException("service binary '%s' is not executable",(serviceDir+'/'+service_exe).c_str());
      }
      if(info.hostName.length()>0)
      {
        if(!checkExec((serviceDir+"/"+hostUp).c_str()))
        {
          throw AdminException("Logical hostname specified, but 'hostup' script not found at '%s'",(serviceDir+"/"+hostUp).c_str());
        }
      }
      if (pid_t p = fork())
      { // parent process
        stguard.release();
        status = running;
        return pid = p;
      }
      else
      { // child process
        chdir(serviceDir.c_str());
        //chmod(service_exe, S_IRWXU | S_IRGRP | S_IXGRP);

        // close all (parent) streams
        struct rlimit flim;
        getrlimit(RLIMIT_NOFILE, &flim);
        for (rlim_t i=0; i<flim.rlim_cur; i++) {
          close((int)i);
        }

        FILE* tmpStream;
        tmpStream = freopen("service.err", "a",  stderr);
        if (!tmpStream)
        {
          smsc_log_error(logger, "reopen stderr error : %s",strerror(errno));
        }
        tmpStream = freopen("service.out", "a",  stdout);
        if (!tmpStream)
        {
          smsc_log_error(logger, "reopen stdout error : %s",strerror(errno));
        }
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set,17);
        sigaddset(&set, SIGBUS);
        sigaddset(&set, SIGFPE);
        sigaddset(&set, SIGILL);
        sigaddset(&set, SIGSEGV);
        sigaddset(&set, SIGALRM);
        sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
        if(pthread_sigmask(SIG_UNBLOCK,&set,NULL)!=0)
        {
                __warning__("Faield to update signal mask");
        }
        if(info.hostName.length()>0)
        {
          std::string cmd=hostUp;
          cmd+=" ";
          cmd+=info.hostName;
          ::system(cmd.c_str());
        }
        execv(service_exe, createArguments());
        smsc_log_error(logger, "Couldn't start service (\"%s/%s\"), nested: %u: %s",
                     serviceDir.c_str(), service_exe, errno, strerror(errno));
        exit(-1);
        return 0;
      }
    }
  default:
    throw AdminException("Unknown service status");
  }
}

void Service::kill()
  throw (AdminException)
{
  sleepEvent.Signal();
  if (status == stopped)
  {
    throw AdminException("Service is not running");
  }

  //int result = sigsend(P_PID, pid, SIGKILL);
  int result = ::kill(pid, SIGKILL);
  if (result != 0)
  {
    switch (errno)
    {
    case EINVAL:
      throw AdminException("Incorrect signal number");
    case EPERM:
      throw AdminException("Does not have permission to send the signal to Service process");
    case ESRCH:
      status = stopped;
      pid = 0;
      throw AdminException("No process or process group can be found corresponding to that specified by pid");
    default:
      throw AdminException("Unknown error");
    }
  }
  status = stopped;
  pid = 0;
}

void Service::shutdown()
  throw (AdminException)
{
  sleepEvent.Signal();
  if (status != running)
  {
    if(status==starting)
    {
      restartRetryCount=0;
      status=stopping;
    }
    throw AdminException("Service is not running");
  }

  smsc_log_debug(logger, "sending %i signal to %u", smsc::system::SHUTDOWN_SIGNAL, pid);
  //int result = sigsend(P_PID, pid, smsc::system::SHUTDOWN_SIGNAL);
  lastStart=0;
  restartRetryCount=0;
  int result = ::kill(pid, smsc::system::SHUTDOWN_SIGNAL);
  if (result != 0)
  {
    switch (errno)
    {
    case EINVAL:
      throw AdminException("Incorrect signal number");
    case EPERM:
      throw AdminException("Does not have permission to send the signal to Service process");
    case ESRCH:
      status = stopped;
      pid = 0;
      throw AdminException("No process or process group can be found corresponding to that specified by pid");
    default:
      throw AdminException("Unknown error");
    }
  }
  status = stopping;
}

void Service::init(const char * const services_dir,
           const ServiceInfo& svcInfo,
           const pid_t servicePID,
           const run_status serviceStatus
           )
{
  info=svcInfo;

  serviceDir=services_dir;
  serviceDir+='/';
  serviceDir+=info.id;

  pid = servicePID;
  status = serviceStatus;
}

char * substr(const char * from, const char * to)
{
  char *tmpbuf = new char[to - from +1];
  memcpy(tmpbuf, from, to - from);
  tmpbuf[to - from] = 0;
  return tmpbuf;
}

char ** Service::createArguments()
{
  typedef std::vector<char *> chrvec;
  chrvec args_vector;
  args_vector.push_back(cStringCopy(service_exe));

  ///////////////////////////////////////
  // parse arguments
  ///////////////////////////////////////
  {
    const char *p1 = info.args.c_str(), *p2 = info.args.c_str();
    while (*p1!= 0)
    {
      if (isspace(*p1))
      {
        if (p1 == p2)
        {
          p2++;
        } else {
          args_vector.push_back(substr(p2, p1));
          p2 = p1+1;
        }
      }
      p1++;
    }
    if (*p2 != 0 && p2 != p1)
      args_vector.push_back(substr(p2, p1));
  }

  char ** arguments = new char*[args_vector.size()+1];
  for (chrvec::size_type i = 0; i < args_vector.size(); i++)
  {
    arguments[i] = args_vector.at(i);
  }
  arguments[args_vector.size()] = 0;
  return arguments;
}

}
}
}
