/*
	$Id$
*/

#if !defined __Cpp_Header__router_types_h__
#define __Cpp_Header__router_types_h__

#include "smeman/smetypes.h"

namespace smsc {
namespace router {

typedef int RoutePriority;
static const RoutePriorityMax = 65000;
static const RoutePriorityMin = 0;

typedef long RouteId;

struct RouteInfo
{
	RoutePriority priority;
	bool billing;
	bool paid;
	bool archived;
	RouteId routeId;
	smsc::smeman::SmeSystemId smeSystemId;
};

}; // namespace router
}; // namespcase smsc

#endif
