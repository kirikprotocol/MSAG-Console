#include "RouteRegistry.hpp"
#include "test/util/Util.hpp"
#include "test/sms/SmsUtil.hpp"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
using smsc::sms::AddressValue;
using namespace smsc::test::util;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using smsc::test::sms::str;

RouteRegistry::RouteIterator::RouteIterator(
	AddressMap::const_iterator i1, AddressMap::const_iterator i2)
	: it(i1), itEnd(i2)
{
	if (it != itEnd)
	{
		it2 = it->second.begin();
	}
}

const RouteHolder* RouteRegistry::RouteIterator::next()
{
	while (it != itEnd)
	{
		while (it2 != it->second.end())
		{
			return (it2++)->second;
		}
		if ((++it != itEnd))
		{
			it2 = it->second.begin();
		}
	}
	return NULL;
}

RouteRegistry::~RouteRegistry()
{
	clear();
}

bool RouteRegistry::putRoute(const RouteInfo& route, SmeProxy* proxy)
{
	//дублированные маршруты не сохраняются
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
	return true;
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
	routeMap.clear();
}

RouteRegistry::RouteIterator* RouteRegistry::iterator() const
{
	return new RouteIterator(addrMap.begin(), addrMap.end());
}

const RouteHolder* RouteRegistry::getRoute(RouteId routeId) const
{
	RouteMap::const_iterator it = routeMap.find(routeId);
	return (it != routeMap.end() ? it->second : NULL);
}

const bool RouteRegistry::findDestAddress(const Address& origAddr, SmeSystemId destSmeId, Address& destAddr) const
{
	for (RouteMap::const_iterator it = routeMap.begin(); it != routeMap.end(); it++)
	{
		const RouteHolder* routeHolder = it->second;
		//совпадение smeId
		if (routeHolder->route.smeSystemId != destSmeId)
		{
			continue;
		}
		//source удовлетворяет маске 
		const Address& origMask = routeHolder->route.source;
		if (origAddr.getTypeOfNumber() != origMask.getTypeOfNumber() ||
			origAddr.getNumberingPlan() != origMask.getNumberingPlan() ||
			origAddr.getLenght() != origMask.getLenght())
		{
			continue;
		}
		const string addrValue = origAddr.value;
		const string maskValue = origMask.value;
		int len = maskValue.find('?');
		if (len == string::npos)
		{
			len = maskValue.length();
		}
		if (addrValue.compare(0, len, maskValue, 0, len))
		{
			continue;
		}
		//подобрать dest
		string destValue = routeHolder->route.dest.value;
		for (int i = 0; i < destValue.length(); i++)
		{
			if (destValue[i] == '?')
			{
				destValue[i] = '0';
			}
		}
		destAddr.setTypeOfNumber(routeHolder->route.dest.getTypeOfNumber());
		destAddr.setNumberingPlan(routeHolder->route.dest.getNumberingPlan());
		destAddr.setValue(destValue.length(), destValue.c_str());
		//проверить
		routeHolder = lookup(origAddr, destAddr);
		if (routeHolder && routeHolder->route.smeSystemId == destSmeId)
		{
			__trace2__("findDestAddress(): destAddr = %s for origAddr = %s, destSmeId = %s", str(destAddr).c_str(), str(origAddr).c_str(), destSmeId.c_str());
			return true;
		}
	}
	__trace2__("findDestAddress(): destAddr not found for origAddr = %s, destSmeId = %s", str(origAddr).c_str(), destSmeId.c_str());
	return false;
}

const RouteHolder* RouteRegistry::lookup2(const AddressMap2& addrMap2,
	const Address& origAddr) const
{
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
		AddressMap2::const_iterator it = addrMap2.find(addr);
		ostringstream os;
		os << addr;
		/*
		__trace2__("RouteRegistry::lookup2(): origAddr = %s, res = %d",
			os.str().c_str(), (it == addrMap2.end() ? 0 : 1));
		*/
		if (it != addrMap2.end())
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
		ostringstream os;
		os << addr;
		/*
		__trace2__("RouteRegistry::lookup1(): destAddr = %s, res = %d",
			os.str().c_str(), (it == addrMap.end() ? 0 : it->second.size()));
		*/
		if (it != addrMap.end())
		{
			const RouteHolder* routeHolder = lookup2(it->second, origAddr);
			if (routeHolder)
			{
				//return routeHolder;
				if (!routeHolder->route.enabling)
				{
					__trace__("Inhibit route found");
				}
				return (routeHolder->route.enabling ? routeHolder : NULL);
			}
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

int RouteRegistry::size() const
{
	return routeMap.size();
}

}
}
}

