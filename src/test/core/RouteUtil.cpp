#include "RouteUtil.hpp"
#include "router/route_types.h"
#include "test/sms/SmsUtil.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::sms::AddressValue;
using smsc::test::sms::SmsUtil;
using namespace smsc::router;
using namespace smsc::test::util;

void RouteUtil::setupRandomCorrectRouteInfo(
	const SmeSystemId& smeSystemId, RouteInfo* route)
{
	route->priority = rand2(RoutePriorityMin, RoutePriorityMax);
	route->billing = rand0(1);
	route->paid = rand0(1);
	route->archived = rand0(1);
	route->routeId = (RouteId) rand0(INT_MAX);
	route->smeSystemId = smeSystemId;
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
	int len;
	AddressValue tmp;
	os << "routeId = " << route.routeId;
	os << ", smeSystemId = " << route.smeSystemId << "(" <<
		route.smeSystemId.length() << ")";
	len = route.source.getValue(tmp);
	os << ", source = " << tmp << "(" << len << ")";
	len = route.dest.getValue(tmp);
	os << ", dest = " << tmp << "(" << len << ")";
	os << ", priority = " << route.priority;
	os << ", billing = " << (route.billing ? "true" : "false");
	os << ", paid = " << (route.paid ? "true" : "false");
	os << ", archived = " << (route.archived ? "true" : "false");
	return os;
}

ostream& operator<< (ostream& os, const TestRouteData& data)
{
	int len;
	AddressValue tmp;
	os << "match = " << (data.match ? "true" : "false");
	len = data.origAddr.getValue(tmp);
	os << ", origAddr = " << tmp << "(" << len << ")";
	len = data.destAddr.getValue(tmp);
	os << ", destAddr = " << tmp << "(" << len << ")";
	os << ", origAddrMatch = " << data.origAddrMatch;
	os << ", destAddrMatch = " << data.destAddrMatch;
	os << ", proxyId = " << data.proxyId;
	os << ", route = {" << *data.route << "}";
	return os;
}

}
}
}

