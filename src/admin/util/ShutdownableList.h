#ifndef SMSC_ADMIN_UTIL_SHUTDOWN_SIGNAL_LISTENER
#define SMSC_ADMIN_UTIL_SHUTDOWN_SIGNAL_LISTENER
/*
#include <list>
#include <core/synchronization/Mutex.hpp>
#include <admin/util/Shutdownable.h>
#include <util/signal.hpp>

using smsc::core::synchronization::Mutex;

namespace smsc {
namespace admin {
namespace util {

class ShutdownableList
{
protected:
	typedef std::list<Shutdownable *> Listeners;
	static Listeners listeners;
	static Mutex shutdownSignalListenerMutex;

public:
	static void addListener(Shutdownable *newListener);
	static void removeListener(Shutdownable *newListener);	
	static void shutdown();
};

}
}
}*/
#endif // ifndef SMSC_ADMIN_UTIL_SHUTDOWN_SIGNAL_LISTENER
