#include "RouteUtil.hpp"
#include "router/route_types.h"
#include "test/util/Util.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using namespace smsc::router;
using namespace smsc::test::util;

void RouteUtil::setupRandomCorrectRouteInfo(RouteInfo* route)
{
	route->priority = rand2(RoutePriorityMin, RoutePriorityMax);
	route->billing = rand0(10);
	route->paid = rand0(10);
	route->archived = rand0(10);
	route->enabling = rand0(2);
	route->routeId = (RouteId) rand1(INT_MAX);
	//route->smeSystemId = ...
	//route->source = ...
	//route->dest = ...
}

vector<int> RouteUtil::compareRoutes(const RouteInfo& route1,
	const RouteInfo& route2)
{
	vector<int> res;
	if (route1.priority != route2.priority)
	{
		res.push_back(1);
	}
	if (route1.billing != route2.billing)
	{
		res.push_back(2);
	}
	if (route1.paid != route2.paid)
	{
		res.push_back(3);
	}
	if (route1.archived != route2.archived)
	{
		res.push_back(4);
	}
	if (route1.routeId != route2.routeId)
	{
		res.push_back(5);
	}
	if (route1.smeSystemId != route2.smeSystemId)
	{
		res.push_back(6);
	}
	if (!SmsUtil::compareAddresses(route1.source, route2.source))
	{
		res.push_back(7);
	}
	if (!SmsUtil::compareAddresses(route1.dest, route2.dest))
	{
		res.push_back(8);
	}
	return res;
}

ostream& operator<< (ostream& os, const RouteInfo& route)
{
	os << "routeId = " << route.routeId;
	os << ", smeSystemId = " << route.smeSystemId << "(" <<
		route.smeSystemId.length() << ")";
	os << ", source = " << route.source;
	os << ", dest = " << route.dest;
	os << ", priority = " << route.priority;
	os << ", billing = " << (route.billing ? "true" : "false");
	os << ", paid = " << (route.paid ? "true" : "false");
	os << ", archived = " << (route.archived ? "true" : "false");
	return os;
}

ostream& operator<< (ostream& os, const RouteHolder& holder)
{
	os << "route = {" << holder.route;
	os << "}, proxy = " << holder.proxy->getUniqueId();
	return os;
}

}
}
}

