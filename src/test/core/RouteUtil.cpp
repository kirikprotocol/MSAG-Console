#include "RouteUtil.hpp"
#include "router/route_types.h"
#include "test/util/Util.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using smsc::test::sms::operator!=;
using namespace std;
using namespace smsc::router;
using namespace smsc::test::util;

void RouteUtil::setupRandomCorrectRouteInfo(RouteInfo* route)
{
	static Mutex mutex;
	static int routeId = 1;
	MutexGuard mguard(mutex);
	route->priority = rand2(RoutePriorityMin, RoutePriorityMax);
	route->billing = rand0(10);
	route->archived = rand0(3);
	//route->enabling = rand0(2);
	route->serviceId = rand0(INT_MAX);
	route->suppressDeliveryReports = false; //todo: rand0(1)
	ostringstream s;
	s << "route_" << routeId++;
	route->routeId = s.str();
	//route->smeSystemId = ...
	//route->source = ...
	//route->dest = ...
}

#define __check__(errCode, field) \
	if (route1.field != route2.field) { res.push_back(errCode); }
	
vector<int> RouteUtil::compareRoutes(const RouteInfo& route1,
	const RouteInfo& route2)
{
	vector<int> res;
	__check__(1, priority);
	__check__(2, billing);
	__check__(3, archived);
	__check__(4, enabling);
	__check__(5, serviceId);
	__check__(6, routeId);
	__check__(7, smeSystemId);
	__check__(8, source);
	__check__(9, dest);
	return res;
}

ostream& operator<< (ostream& os, const RouteInfo& route)
{
	os << "routeId = " << route.routeId;
	os << ", smeSystemId = " << route.smeSystemId;
	os << ", source = " << route.source;
	os << ", dest = " << route.dest;
	os << ", enabling = " << route.enabling;
	os << ", priority = " << route.priority;
	os << ", billing = " << (route.billing ? "true" : "false");
	os << ", archived = " << (route.archived ? "true" : "false");
	os << ", serviceId = " << route.serviceId;
	return os;
}

ostream& operator<< (ostream& os, const RouteHolder& holder)
{
	os << "route = {" << holder.route;
	if (holder.proxy)
	{
		os << "}, proxy = " << holder.proxy->getUniqueId();
	}
	else
	{
		os << "}, proxy = NULL";
	}
	return os;
}

const string str(const RouteInfo& route)
{
	ostringstream os;
	os << route;
	return os.str();
}

const string str(const RouteHolder& holder)
{
	ostringstream os;
	os << holder;
	return os.str();
}

}
}
}

