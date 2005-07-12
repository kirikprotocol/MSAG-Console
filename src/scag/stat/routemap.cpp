#include "routemap.h"

namespace scag {
namespace stat {

using smsc::core::buffers::Hash;
using smsc::core::buffers::IntHash;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

RouteMap::RouteMap()
{
	routeId = 1;
}

RouteMap::~RouteMap()
{
}

int RouteMap::regRoute(const char *routeId, int &newRouteId)
{
	MutexGuard mg(mutex);
	newRouteId = -1;
	int *p = intByStr.GetPtr(routeId);
	if(!p){
		intByStr.Insert(routeId, ++routeId_);
        strByInt.Insert(routeId_, std::string(routeId));
		newRouteId = routeId_;
		return -1;
	}else{
		return *p;
	}
}

char *RouteMap::lookup(int routeId)
{
	MutexGuard mg(mutex);
	std::string *p = strByInt.GetPtr(routeId);
	if(!p)
		return 0;
		
	return p->c_str();
}

int RouteMap::lookup(const char *routeId)
{
	MutexGuard mg(mutex);
	int *p = intByStr.GetPtr(routeId);
	if(!p)
		return -1;
		
	return *p;
}

}
}

