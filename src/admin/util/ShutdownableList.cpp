#include "ShutdownableList.h"

#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace admin {
namespace util {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

ShutdownableList::Listeners ShutdownableList::listeners;
Mutex ShutdownableList::shutdownSignalListenerMutex;

void ShutdownableList::addListener(Shutdownable *newListener)
{
	MutexGuard a(shutdownSignalListenerMutex);
	listeners.push_back(newListener);
}

void ShutdownableList::removeListener(Shutdownable *newListener)
{
	MutexGuard a(shutdownSignalListenerMutex);
	listeners.remove(newListener);
}

void ShutdownableList::shutdown()
{
	MutexGuard a(shutdownSignalListenerMutex);
	for (Listeners::iterator i = listeners.begin(); i != listeners.end(); i++)
	{
		Shutdownable * listener = *i;
		listener->shutdown();
	}
}

}
}
}
