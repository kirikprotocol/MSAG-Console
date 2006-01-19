#include "scag/scagsignalhandlers.h"

#include "scag/scag.h"
#include "admin/service/ServiceSocketListener.h"

#include <pthread.h>

namespace scag 
{

using scag::Scag;
using namespace scag::admin;

Scag * _scag = 0;
SCAGSocketListener* _socketListener = 0;

void clearThreadSignalMask()
{
  sigset_t set;
  sigemptyset(&set);
  for(int i=1;i<=37;i++)if(i!=SIGQUIT)sigaddset(&set,i);
  if(pthread_sigmask(SIG_SETMASK,&set,NULL)!=0)
  {
    __warning__("failed to set thread signal mask!");
  };
}

extern "C" void sigAbortDispatcher(int sig)
{
  fprintf(stderr, "Signal %i received, abort\n", sig);
  if (_scag != 0)
  {
    _scag->abortScag();
  }
  else
  {
    if (_socketListener != 0)
      _socketListener->abort();
    SCAGCommandDispatcher::abortGw();
  }
}

extern "C" void sigDumpDispatcher(int sig)
{
  fprintf(stderr, "Signal %i received, dump & shutdown\n", sig);
  if (_scag != 0)
    _scag->dumpScag();
  else
  {
    if (_socketListener != 0)
      _socketListener->abort();
    SCAGCommandDispatcher::dumpGw();
  }
}

extern "C" void sigShutdownHandler(int signo)
{
  fprintf(stderr, "Signal %i received, shutdown\n", signo);
  if (_scag != 0)
    _scag->shutdown();
  else
  {
    if (_socketListener != 0)
      _socketListener->shutdown();
    SCAGCommandDispatcher::stopGw();
  }
}

void registerSignalHandlers_internal()
{
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set,17);
  sigaddset(&set, SIGBUS);
  sigaddset(&set, SIGFPE);
  sigaddset(&set, SIGILL);
  sigaddset(&set, SIGSEGV);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGABRT);
  sigaddset(&set, SHUTDOWN_SIGNAL);
//#ifndef SPARC
//  sigaddset(&set,SIGQUIT);
//#endif

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
  sigset(SHUTDOWN_SIGNAL,  sigShutdownHandler);
}

void registerScagSignalHandlers(Scag * scag)
{
  _scag = scag;
  registerSignalHandlers_internal();
}

void registerScagSignalHandlers(SCAGSocketListener * socketListener)
{
  _socketListener = socketListener;
  registerSignalHandlers_internal();
}

}
