#include "SignalHandler.h"

#include <core/synchronization/Mutex.hpp>
#include <util/signal.h>

namespace smsc {
namespace admin {
namespace util {

smsc::core::synchronization::Mutex SignalHandler::shutdownLock;
SignalHandler * SignalHandler::shutdownHandler = 0;

void SignalHandler::registerShutdownHandler(SignalHandler * handler) throw()
{
	smsc::core::synchronization::MutexGuard guard(shutdownLock);
	shutdownHandler = handler;
	if (handler != 0)
	{
		smsc::util::setSignalHandler(SHUTDOWN_SIGNAL, shutdownSignalHandler);
	}
}

void SignalHandler::shutdownSignalHandler(int signo)
{
	smsc::core::synchronization::MutexGuard guard(shutdownLock);
	if (shutdownHandler != 0)
		shutdownHandler->handleSignal();
}

}
}
}

