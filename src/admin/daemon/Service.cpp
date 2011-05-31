#include "Service.h"

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <system/smscsignalhandlers.h>
#include <sys/resource.h>

namespace smsc {
namespace admin {
namespace daemon {

const char * const Service::service_exe = "bin/service";

pid_t Service::start()
  throw (AdminException)
{
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
      if (pid_t p = fork())
      { // parent process
        status = running;
        return pid = p;
      }
      else
      { // child process
        chdir(service_dir.get());
        chmod(service_exe, S_IRWXU | S_IRGRP | S_IXGRP);

        // close all (parent) streams
        struct rlimit flim;
        getrlimit(RLIMIT_NOFILE, &flim);
        for (rlim_t i=0; i<flim.rlim_cur; i++) {
          close(i);
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
        if(pthread_sigmask(SIG_UNBLOCK,&set,NULL)!=0) {
                __warning__("Faield to update signal mask");
        }
        execv(service_exe, createArguments());
        smsc_log_error(logger, "Couldn't start service (\"%s/%s\"), nested: %u: %s",
                     service_dir.get(), service_exe, errno, strerror(errno));
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
  if (status != running)
  {
    throw AdminException("Service is not running");
  }

  smsc_log_debug(logger, "sending %i signal to %u", smsc::system::SHUTDOWN_SIGNAL, pid);
  //int result = sigsend(P_PID, pid, smsc::system::SHUTDOWN_SIGNAL);
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
           const char * const serviceId,
           const char * const serviceArgs,
           const pid_t servicePID,
                   const run_status serviceStatus)
{
  service_dir.reset(new char[strlen(services_dir) + 1 + strlen(serviceId) + 1]);
  strcpy(service_dir.get(), services_dir);
  strcat(service_dir.get(), "/");
  strcat(service_dir.get(), serviceId);

  id.reset(cStringCopy(serviceId));
  pid = servicePID;
  status = serviceStatus;
  args.reset(cStringCopy(serviceArgs));
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
    const char *p1 = args.get(), *p2 = args.get();
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
