#include "RouteRegistry.hpp"
#include "test/util/Util.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace std;
using namespace smsc::test::util;
using smsc::test::sms::SmsUtil;

RouteRegistry::RouteIterator::RouteIterator(RouteMap::iterator b, 
	RouteMap::iterator e) : it(b), end(e) {}

bool RouteRegistry::RouteIterator::hasNext() const
{
	return (it != end);
}

const TestRouteData* RouteRegistry::RouteIterator::operator*() const
{
	return it->second;
}

const TestRouteData* RouteRegistry::RouteIterator::operator->() const
{
	return it->second;
}

RouteRegistry::RouteIterator& RouteRegistry::RouteIterator::operator++()
{
	it++;
	return *this;
}

RouteRegistry::RouteIterator RouteRegistry::RouteIterator::operator++(int)
{
	it++;
	return *this;
}

RouteRegistry::~RouteRegistry()
{
	for (RouteMap::iterator it = routeMap.begin(); it != routeMap.end(); it++)
	{
		delete it->second;
	}
}

void RouteRegistry::putRoute(const TestRouteData& data)
{
	if (data.route)
	{
		TestRouteData* routeData = new TestRouteData(data);
		addrMap[data.origAddr].push_back(routeData);
		routeMap[data.route->routeId] = routeData;
	}
}

const RouteInfo* RouteRegistry::getRoute(RouteId routeId) const
{
	RouteMap::const_iterator it = routeMap.find(routeId);
	if (it != routeMap.end())
	{
		return it->second->route;
	}
	return NULL;
}

RouteRegistry::RouteIterator* RouteRegistry::iterator()
{
	return new RouteIterator(routeMap.begin(), routeMap.end());
}

int RouteRegistry::size() const
{
	return routeMap.size();
}

const Address* RouteRegistry::getRandomReachableDestAddress(
	const Address& origAddr) const
{
	AddressMap::const_iterator it = addrMap.find(origAddr);
	if (it == addrMap.end())
	{
		return NULL;
	}
	const RouteList& routes = it->second;
	RouteList res;
	for (int i = 0; i < routes.size(); i++)
	{
		if (routes[i]->match)
		{
			res.push_back(routes[i]);
		}
	}
	return (res.size() ? new Address(res[rand0(res.size() - 1)]->destAddr) : NULL);
}

const Address* RouteRegistry::getRandomNonReachableDestAddress(
	const Address& origAddr) const
{
	AddressMap::const_iterator it = addrMap.find(origAddr);
	if (it == addrMap.end())
	{
		return NULL;
	}
	const RouteList& routes = it->second;
	RouteList res;
	for (int i = 0; i < routes.size(); i++)
	{
		if (!routes[i]->match)
		{
			res.push_back(routes[i]);
		}
	}
	return (res.size() ? new Address(res[rand0(res.size() - 1)]->destAddr) : NULL);
}

const RouteRegistry::RouteList RouteRegistry::lookup(const Address& origAddr,
	const Address& destAddr) const
{
	RouteList res;
	AddressMap::const_iterator it = addrMap.find(origAddr);
	if (it == addrMap.end())
	{
		return res;
	}
	const RouteList& routes = it->second;
	//приоритеты маршрутов: destAddr, origAddr
	for (int i = 0; i < routes.size(); i++)
	{
		//общие проверки
		if (!routes[i]->match ||
			!SmsUtil::compareAddresses(destAddr, routes[i]->destAddr))
		{
			continue;
		}
		if (!res.size())
		{
			res.push_back(routes[i]);
			continue;
		}
		//destAddr
		if (res.back()->destAddrMatch < routes[i]->destAddrMatch)
		{
			res.clear();
			res.push_back(routes[i]);
			continue;
		}
		if (res.back()->destAddrMatch > routes[i]->destAddrMatch)
		{
			continue;
		}
		//origAddr
		if (res.back()->origAddrMatch < routes[i]->origAddrMatch)
		{
			res.clear();
			res.push_back(routes[i]);
			continue;
		}
		if (res.back()->origAddrMatch == routes[i]->origAddrMatch)
		{
			res.push_back(routes[i]);
			continue;
		}
	}
	return res;
}

}
}
}

