#ifndef TEST_CORE_ROUTE_UTIL
#define TEST_CORE_ROUTE_UTIL

#include "sms/sms.h"
#include "router/route_types.h"
#include "smeman/smetypes.h"
#include "smeman/smeproxy.h"
#include <ostream>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::ostream;
using std::vector;
using smsc::sms::Address;
using smsc::router::RouteInfo;
using smsc::smeman::SmeSystemId;
using smsc::smeman::SmeProxy;

struct TestRouteData
{
	bool match;
	float origAddrMatch;
	float destAddrMatch;
	const Address origAddr;
	const Address destAddr;
	const SmeProxy* proxy;
	RouteInfo* route;

	TestRouteData(const Address& _origAddr, const Address& _destAddr,
		SmeProxy* _proxy = NULL)
		: match(false), origAddrMatch(0.0), destAddrMatch(0.0),
		origAddr(_origAddr), destAddr(_destAddr),
		proxy(_proxy), route (NULL) {}

	TestRouteData(const TestRouteData& data)
		: match(data.match), origAddrMatch(data.origAddrMatch),
		destAddrMatch(data.destAddrMatch), origAddr(data.origAddr),
		destAddr(data.destAddr), proxy(data.proxy),
		route(NULL)
	{
		if (data.route)
		{
			route = new RouteInfo(*data.route);
		}
	}

	~TestRouteData()
	{
		if (route)
		{
			delete route;
		}
		//delete proxy;
	}
};

class RouteUtil
{
public:
	static void setupRandomCorrectRouteInfo(const SmeSystemId& smeSystemId,
		RouteInfo* info);

	static vector<int> compareRoutes(const RouteInfo& route1,
		const RouteInfo& route2);
};

ostream& operator<< (ostream& os, const RouteInfo& route);
ostream& operator<< (ostream& os, const TestRouteData& data);

}
}
}

#endif /* TEST_CORE_ROUTE_UTIL */

