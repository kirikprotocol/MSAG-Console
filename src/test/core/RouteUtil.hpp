#ifndef TEST_CORE_ROUTE_UTIL
#define TEST_CORE_ROUTE_UTIL

#include "sms/sms.h"
#include "router/route_types.h"
#include <ostream>

namespace smsc {
namespace test {
namespace core {

using std::ostream;
using smsc::sms::Address;
using smsc::router::RouteInfo;

struct TestRouteData
{
	bool match;
	float origAddrMatch;
	float destAddrMatch;
	const Address origAddr;
	const Address destAddr;
	RouteInfo* route;

	TestRouteData(const Address& _origAddr, const Address& _destAddr)
		: match(true), origAddrMatch(0.0), destAddrMatch(0.0),
		origAddr(_origAddr), destAddr(_destAddr), route(NULL) {}

	~TestRouteData()
	{
		if (route)
		{
			delete route;
		}
	}
};

ostream& operator<< (ostream& os, const RouteInfo& route);
ostream& operator<< (ostream& os, const TestRouteData& data);

}
}
}

#endif /* TEST_CORE_ROUTE_UTIL */

