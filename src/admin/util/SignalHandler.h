#ifndef SMSC_ADMIN_SERVICE_SIGNAL_HANDLER
#define SMSC_ADMIN_SERVICE_SIGNAL_HANDLER

#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace admin {
namespace util {

class SignalHandler
{
public:
	static const int SHUTDOWN_SIGNAL = SIGINT;
	
	virtual void handleSignal() throw() = 0;
	static void registerShutdownHandler(SignalHandler * handler) throw();

protected:
	
	static SignalHandler * shutdownHandler;
	static smsc::core::synchronization::Mutex shutdownLock;
	static void shutdownSignalHandler(int signo);
};

}
}
}
#endif
