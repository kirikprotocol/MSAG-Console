#include "RouteRegistry.hpp"
#include "test/util/Util.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace smsc::test::util;
using smsc::test::sms::SmsUtil;

RouteRegistry::~RouteRegistry()
{
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		RouteList routes = it->second;
		for (int i = 0; i < routes.size(); i++)
		{
			delete routes[i];
		}
	}
}

void RouteRegistry::putRoute(const TestRouteData& data)
{
	addrMap[data.origAddr].push_back(new TestRouteData(data));
}

const Address* RouteRegistry::getRandomReachableDestAddress(const Address& origAddr)
{
	vector<const TestRouteData*>& routes = addrMap[origAddr];
	vector<const TestRouteData*> res;
	for (int i = 0; i < routes.size(); i++)
	{
		if (routes[i]->match)
		{
			res.push_back(routes[i]);
		}
	}
	return (res.size() ? new Address(res[rand0(res.size() - 1)]->destAddr) : NULL);
}

const Address* RouteRegistry::getRandomNonReachableDestAddress(const Address& origAddr)
{
	vector<const TestRouteData*>& routes = addrMap[origAddr];
	vector<const TestRouteData*> res;
	for (int i = 0; i < routes.size(); i++)
	{
		if (!routes[i]->match)
		{
			res.push_back(routes[i]);
		}
	}
	return (res.size() ? new Address(res[rand0(res.size() - 1)]->destAddr) : NULL);
}

const SmeSystemId* RouteRegistry::lookup(const Address& origAddr,
	const Address& destAddr)
{
	vector<const TestRouteData*>& routes = addrMap[origAddr];
	//приоритеты маршрутов: destAddr, origAddr, priority
	int idx = -1;
	for (int i = 0; i < routes.size(); i++)
	{
		if (!routes[i]->match)
		{
			continue;
		}
		if (!SmsUtil::compareAddresses(destAddr, routes[i]->destAddr))
		{
			continue;
		}
		if (idx < 0)
		{
			idx = i;
			continue;
		}
		if (routes[idx]->destAddrMatch < routes[i]->destAddrMatch)
		{
			idx = i;
			continue;
		}
		if (routes[idx]->destAddrMatch == routes[i]->destAddrMatch)
		{
			if (routes[idx]->origAddrMatch < routes[i]->origAddrMatch)
			{
				idx = i;
				continue;
			}
			if (routes[idx]->origAddrMatch == routes[i]->origAddrMatch &&
				routes[idx]->route->priority < routes[i]->route->priority)
			{
				idx = i;
				continue;
			}
		}
	}
	return (idx < 0 ? NULL : &routes[idx]->route->smeSystemId);
}

}
}
}

