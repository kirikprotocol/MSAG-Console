#ifndef TEST_CORE_ROUTE_UTIL
#define TEST_CORE_ROUTE_UTIL

#include "sms/sms.h"
#include "router/route_types.h"
#include "smeman/smetypes.h"
#include "smeman/smeproxy.h"
#include <ostream>
#include <vector>
#include <string>

namespace smsc {
namespace test {
namespace core {

using std::ostream;
using std::vector;
using std::string;
using smsc::sms::Address;
using smsc::router::RouteInfo;
using smsc::smeman::SmeSystemId;
using smsc::smeman::SmeProxy;

struct RouteHolder
{
	const RouteInfo route;
	const SmeProxy* proxy;
	
	RouteHolder(const RouteInfo& routeInfo, const SmeProxy* smeProxy)
		: route(routeInfo), proxy(smeProxy) {}
	~RouteHolder() {}
};

class RouteUtil
{
public:
	static void setupRandomCorrectRouteInfo(RouteInfo* info);

	static vector<int> compareRoutes(const RouteInfo& route1,
		const RouteInfo& route2);
};

ostream& operator<< (ostream& os, const RouteInfo& route);
ostream& operator<< (ostream& os, const RouteHolder& holder);

const string str(const RouteInfo& route);
const string str(const RouteHolder& holder);

}
}
}

#endif /* TEST_CORE_ROUTE_UTIL */

