#ifndef TEST_CORE_CORE_TEST_MANAGER
#define TEST_CORE_CORE_TEST_MANAGER

#include "smeman/smeman.h"
#include "router/route_manager.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::smeman::SmeManager;
using smsc::router::RouteManager;
using smsc::core::synchronization::Mutex;

/**
 * @author bryz
 */
class CoreTestManager
{
public:
	static SmeManager* getSmeManager();
	static RouteManager* getRouteManager();

private:
	static bool inited;
	static SmeManager* smeMan;
    static RouteManager* routeMan;
	
	static void init();
};

}
}
}

#endif /* TEST_CORE_CORE_TEST_MANAGER */

