#include "CoreTestManager.hpp"
#include <cstdlib>
#include <ctime>

namespace smsc {
namespace test {
namespace core {

using smsc::smeman::SmeManager;
using smsc::router::RouteManager;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

SmeManager* CoreTestManager::smeMan = NULL;
RouteManager* CoreTestManager::routeMan = NULL;

void CoreTestManager::init()
{
	static Mutex lock;
	MutexGuard mguard(lock);
	if (!smeMan && !routeMan)
	{
		smeMan = new SmeManager();
		routeMan = new RouteManager();
		routeMan->assign(smeMan);
	}
}

SmeManager* CoreTestManager::getSmeManager()
{
	init();
	return smeMan;
}

RouteManager* CoreTestManager::getRouteManager()
{
	init();
	return routeMan;
}

}
}
}

