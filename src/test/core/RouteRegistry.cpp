#include "RouteRegistry.hpp"
#include "test/util/Util.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace std;
using smsc::sms::AddressValue;
using namespace smsc::test::util;
using smsc::test::sms::SmsUtil;

RouteRegistry::~RouteRegistry()
{
	clear();
}

bool RouteRegistry::putRoute(const RouteInfo& route, SmeProxy* proxy)
{
	//дублированные маршруты не сохран€ютс€
	AddressMap::const_iterator it = addrMap.find(route.dest);
	if (it != addrMap.end())
	{
		if (it->second.find(route.source) != it->second.end())
		{
			return false;
		}
	}
	RouteHolder* routeHolder = new RouteHolder(route, proxy);
	addrMap[routeHolder->route.dest][routeHolder->route.source] = routeHolder;
	routeMap[routeHolder->route.routeId] = routeHolder;
}

void RouteRegistry::clear()
{
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		AddressMap2 addrMap2 = it->second;
		for (AddressMap2::iterator it2 = addrMap2.begin(); it2 != addrMap2.end(); it2++)
		{
			delete it2->second;
		}
		//addrMap2.clear();
	}
	addrMap.clear();
}

const RouteHolder* RouteRegistry::getRoute(RouteId routeId) const
{
	RouteMap::const_iterator it = routeMap.find(routeId);
	return (it != routeMap.end() ? it->second : NULL);
}

const RouteRegistry::AddressMap2* RouteRegistry::lookup1(
	const Address& destAddr) const
{
	Address addr(destAddr);
	AddressValue addrVal;
	int addrLen = addr.getValue(addrVal);
	for (int len = 0; len <= addrLen; len++)
	{
		if (len)
		{
			addrVal[addrLen - len] = '?';
			addr.setValue(addrLen, addrVal);
		}
		AddressMap::const_iterator it = addrMap.find(addr);
		if (it != addrMap.end())
		{
			return &it->second;
		}
		/*
		if (addrLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			addrVal[addrLen - len] = '*';
			addr.setValue(addrLen - len + 1, addrVal);
			AddressMap::iterator it = addrMap.find(addr);
			if (it != addrMap.end())
			{
				return &it->second;
			}
		}
		*/
	}
	return NULL;
}

const RouteHolder* RouteRegistry::lookup2(const AddressMap2* addrMap2,
	const Address& origAddr) const
{
	__require__(addrMap2);
	Address addr(origAddr);
	AddressValue addrVal;
	int addrLen = addr.getValue(addrVal);
	for (int len = 0; len <= addrLen; len++)
	{
		if (len)
		{
			addrVal[addrLen - len] = '?';
			addr.setValue(addrLen, addrVal);
		}
		AddressMap2::const_iterator it = addrMap2->find(addr);
		if (it != addrMap2->end())
		{
			return it->second;
		}
		/*
		if (addrLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			addrVal[addrLen - len] = '*';
			addr.setValue(addrLen - len + 1, addrVal);
			AddressMap2::const_iterator it = addrMap2.find(addr);
			if (it != addrMap2.end())
			{
				return it->second;
			}
		}
		*/
	}
	return NULL;
}

const RouteHolder* RouteRegistry::lookup(const Address& origAddr,
	const Address& destAddr) const
{
	const AddressMap2* addrMap2 = lookup1(destAddr);
	if (addrMap2)
	{
		return lookup2(addrMap2, origAddr);
	}
	return NULL;
}

int RouteRegistry::size() const
{
	return routeMap.size();
}

}
}
}

