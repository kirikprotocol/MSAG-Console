#include <system/smscsignalhandlers.h>
#include <system/smsc.hpp>
#include <admin/smsc_service/SmscComponent.h>
#include <admin/service/ServiceSocketListener.h>

namespace smsc {
namespace system {

using smsc::system::Smsc;
using smsc::admin::smsc_service::SmscComponent;
using smsc::admin::service::ServiceSocketListener;

Smsc * _smsc = 0;
SmscComponent * _smscComponent = 0;
ServiceSocketListener* _socketListener = 0;

void clearThreadSignalMask()
{
	sigset_t set;
	sigemptyset(&set);
	for(int i=1;i<=37;i++)if(i!=SIGQUIT)sigaddset(&set,i);
	if(thr_sigsetmask(SIG_SETMASK,&set,NULL)!=0)
	{
		__warning__("failed to set thread signal mask!");
	};
}

void sigAbortDispatcher(int sig)
{
	fprintf(stderr, "Signal %i received, abort\n", sig);
	if (_smsc != 0)
	{
		_smsc->abortSmsc();
	}
	else
	{
		if (_socketListener != 0)
			_socketListener->abort();
		if (_smscComponent != 0)
			_smscComponent->abort();
	}
}

void sigDumpDispatcher(int sig)
{
	fprintf(stderr, "Signal %i received, dump & shutdown\n", sig);
	if (_smsc != 0)
		_smsc->dumpSmsc();
	else
	{
		if (_socketListener != 0)
			_socketListener->abort();
		if (_smscComponent != 0)
			_smscComponent->dump();
	}
}

void sigShutdownHandler(int signo)
{
	fprintf(stderr, "Signal %i received, shutdown\n", signo);
	if (_smsc != 0)
		_smsc->shutdown();
	else
	{
		if (_socketListener != 0)
			_socketListener->shutdown();
		if (_smscComponent != 0)
			_smscComponent->stopSmsc();
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
	sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
//#ifndef SPARC
//	sigaddset(&set,SIGQUIT);
//#endif

	if(thr_sigsetmask(SIG_UNBLOCK,&set,NULL)!=0)
	{
		__warning__("Faield to update signal mask");
	}
	sigset(17,      sigAbortDispatcher);
	sigset(SIGBUS,  sigDumpDispatcher);
	sigset(SIGFPE,  sigDumpDispatcher);
	sigset(SIGILL,  sigDumpDispatcher);
	sigset(SIGSEGV, sigDumpDispatcher);
  sigset(SIGALRM, sigDumpDispatcher);
	sigset(smsc::system::SHUTDOWN_SIGNAL,  sigShutdownHandler);
}

void registerSmscSignalHandlers(Smsc * smsc)
{
	_smsc = smsc;
	registerSignalHandlers_internal();
}

void registerSmscSignalHandlers(SmscComponent * smscComponent, ServiceSocketListener* socketListener)
{
	_smscComponent = smscComponent;
	_socketListener = socketListener;
	registerSignalHandlers_internal();
}

}
}

