#include "smscsignalhandlers.h"
#include "smsc.hpp"
#include <pthread.h>

namespace smsc {

using smsc::Smsc;

Smsc * _smsc = 0;

void clearThreadSignalMask()
{
  sigset_t set;
  sigemptyset(&set);
  for(int i=1;i<=37;i++)if(i!=SIGQUIT && i!=SIGPROF)sigaddset(&set,i);

  if(pthread_sigmask(SIG_SETMASK,&set,NULL)!=0)
  {
    __warning__("failed to set thread signal mask!");
  };

}

extern "C" void sigAbortDispatcher(int sig)
{
  fprintf(stderr, "Signal %i received, abort\n", sig);
  if (_smsc != 0)
  {
    _smsc->abortSmsc();
  }
}

extern "C" void sigDumpDispatcher(int sig)
{
  fprintf(stderr, "Signal %i received, dump & shutdown\n", sig);
  if (_smsc != 0)
  {
    _smsc->dumpSmsc();
  }
}

extern "C" void sigShutdownHandler(int signo)
{
  fprintf(stderr, "Signal %i received, shutdown\n", signo);
  static time_t firstSig=time(NULL);
  if(_smsc==0)
  {
    if(time(NULL)-firstSig<30)
    {
      fprintf(stderr,"2nd shutdown signal received. 30 seconds not passed since last signal, ignoring\n");
      return;
    }else
    {
      fprintf(stderr,"2nd shutdown signal received. something wrong. aborting.\n");
      abort();
    }
  }


  if (_smsc != 0)
  {
    _smsc->stop();
  }
  _smsc=0;
}

void registerSignalHandlers_internal()
{
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set,1);//for leakhunter
  sigaddset(&set,17);
  sigaddset(&set, SIGBUS);
  sigaddset(&set, SIGFPE);
  sigaddset(&set, SIGILL);
  sigaddset(&set, SIGSEGV);
  sigaddset(&set, SIGALRM);
  sigaddset(&set, SIGABRT);
  sigaddset(&set,SIGPROF);
  sigaddset(&set, smsc::SHUTDOWN_SIGNAL);

  if(pthread_sigmask(SIG_UNBLOCK,&set,NULL)!=0)
  {
    __warning__("Faield to update signal mask");
  }
  sigset(17,      sigAbortDispatcher);
  sigset(SIGBUS,  sigDumpDispatcher);
  sigset(SIGFPE,  sigDumpDispatcher);
  sigset(SIGILL,  sigDumpDispatcher);
  sigset(SIGSEGV, sigDumpDispatcher);
  signal(SIGABRT, sigDumpDispatcher);
  sigset(SIGALRM, sigDumpDispatcher);
  sigset(smsc::SHUTDOWN_SIGNAL,  sigShutdownHandler);
}

void registerSmscSignalHandlers(Smsc * smsc)
{
  _smsc = smsc;
  registerSignalHandlers_internal();
}

}
