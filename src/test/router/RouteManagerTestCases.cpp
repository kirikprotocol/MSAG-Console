#include "RouteManagerTestCases.hpp"
#include "test/core/CoreTestManager.hpp"
#include "test/smeman/SmeManagerUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/core/RouteUtil.hpp"
#include "test/util/Util.hpp"
#include "util/debug.h"
#include <sys/types.h>
#include <sstream>
#include <util/debug.h>

namespace smsc {
namespace test {
namespace router {

using namespace std;
using namespace smsc::sms; //constants, AddressValue
using namespace smsc::router; //constants
using namespace smsc::test::sms; //SmsUtil, print Address
using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, TestSmeProxy
using namespace smsc::test::core; //route utils, CoreTestManager
using smsc::util::Logger;
using smsc::sms::AddressValue;
using smsc::test::core::RouteUtil;

RouteManagerTestCases::RouteManagerTestCases(RouteManager* _routeMan,
	RouteRegistry* _routeReg)
	: routeMan(_routeMan), routeReg(_routeReg)
{
	__require__(routeMan);
	__require__(routeReg);
}

Category& RouteManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("RouteManagerTestCases");
	return log;
}

float RouteManagerTestCases::setupRandomAddressMatch(Address& addr, int num)
{
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	switch(num)
	{
		case 1: //адрес без знаков подстановки (совпадает)
			return 100;
		case 2: //адрес с одним или всеми '?' в конце
			memset(addrVal + addrLen - len, '?', len);
			addr.setValue(addrLen, addrVal);
			return (100 - len);
		case 3: //адрес целиком из '?'
			memset(addrVal, '?', addrLen);
			addr.setValue(addrLen, addrVal);
			return (100 - addrLen);
		/*
		case 4: //адрес со '*' в конце
			memset(addrVal + addrLen - len, '*', 1);
			addr.setValue(addrLen - len + 1, addrVal);
			return (100 - len - 0.5); //'*' менее приоритетна '?'
		case 5: //адрес из '*'
			addrVal[0] = '*';
			addr.setValue(1, addrVal);
			return (100 - addrLen - 0.5);
		case 6: //адрес с лишней '*'
			if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
			{
				addrVal[addrLen] = '*';
				addr.setValue(addrLen + 1, addrVal);
				return 99.5;
			}
			return 100;
		*/
		default:
			throw "";
	}
}

void RouteManagerTestCases::setupRandomAddressNotMatch(Address& addr, int num)
{
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	switch(num)
	{
		case 1: //адрес с лишними '?'
			if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
			{
				memset(addrVal + addrLen - len, '?', len + 1);
				addr.setValue(addrLen + 1, addrVal);
				break;
			}
			//break;
		case 2: //отличающийся адрес
			addrVal[len - 1] = '+';
			addr.setValue(addrLen, addrVal);
			break;
		case 3: //адрес с несовпадающим typeOfNumber
			addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
			break;
		case 4: //адрес с несовпадающим numberingPlan
			addr.setNumberingPlan(addr.getNumberingPlan() + 1);
			break;
		default:
			throw "";
	}
}

TCResult* RouteManagerTestCases::addCorrectRouteMatch(
	const SmeSystemId& smeSystemId, TestRouteData* data, int num)
{
	int numMatch1 = 3; int numMatch2 = 3;
	TCSelector s(num, numMatch1 * numMatch2);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = true;
		data->route = route;
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(smeSystemId, route);
			//origAddr
			route->source = data->origAddr;
			data->origAddrMatch =
				setupRandomAddressMatch(route->source, s.value1(numMatch1));
			//destAddr
			route->dest = data->destAddr;
			data->destAddrMatch =
				setupRandomAddressMatch(route->dest, s.value2(numMatch1));
			getLog().debugStream() << "[" << thr_self() <<
				"]\taddCorrectRouteMatch(" << s.value1(numMatch1) << "," <<
				s.value2(numMatch1) << "): " << *data;
			routeMan->addRoute(*route);
			routeReg->putRoute(*data);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(1000);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRouteNotMatch(
	const SmeSystemId& smeSystemId, TestRouteData* data, int num)
{
	int numMatch = 3; int numNotMatch = 4; int numType = 2;
	TCSelector s(num, numMatch * numNotMatch * numType);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_NOT_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = false;
		data->route = route;
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(smeSystemId, route);
			route->source = data->origAddr;
			route->dest = data->destAddr;
			route->priority = RoutePriorityMax;
			switch (s.value3(numMatch, numNotMatch))
			{
				case 1: //отличается origAddr
					data->origAddrMatch = 0;
					setupRandomAddressNotMatch(route->source,
						s.value1(numNotMatch, numMatch));
					data->destAddrMatch = setupRandomAddressMatch(route->dest,
						s.value2(numNotMatch, numMatch));
					getLog().debugStream() << "[" << thr_self() <<
						"]\taddCorrectRouteNotMatch(" <<
						s.value1(numNotMatch, numMatch) << "," <<
						s.value2(numNotMatch, numMatch) << "," <<
						s.value3(numMatch, numNotMatch) << "): " << *data;
					break;
				case 2: //отличается destAddr
					data->origAddrMatch = setupRandomAddressMatch(route->source,
						s.value1(numMatch, numNotMatch));
					data->destAddrMatch = 0;
					setupRandomAddressNotMatch(route->dest,
						s.value2(numMatch, numNotMatch));
					getLog().debugStream() << "[" << thr_self() <<
						"]\taddCorrectRouteNotMatch(" <<
						s.value1(numMatch, numNotMatch) << "," <<
						s.value2(numMatch, numNotMatch) << "," <<
						s.value3(numMatch, numNotMatch) << "): " << *data;
					break;
				default:
					throw s;
			}
			routeMan->addRoute(*route);
			routeReg->putRoute(*data);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(1000);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRouteNotMatch2(
	const SmeSystemId& smeSystemId, TestRouteData* data, int num)
{
	TCSelector s(num, 4, 1000);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_NOT_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = false;
		data->origAddrMatch = 0;
		data->destAddrMatch = 0;
		data->route = route;
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(smeSystemId, route);
			SmsUtil::setupRandomCorrectAddress(&route->source);
			SmsUtil::setupRandomCorrectAddress(&route->dest);
			switch(s.value())
			{
				case 1001: //origAddr.typeOfNumber вне диапазона
					route->source.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 1002: //origAddr.numberingPlan вне диапазона
					route->source.setNumberingPlan(rand2(0x13, 0xff));
					break;
				case 1003: //destAddr.typeOfNumber вне диапазона
					route->dest.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 1004: //destAddr.numberingPlan вне диапазона
					route->dest.setNumberingPlan(rand2(0x13, 0xff));
					break;
				default:
					throw s;
			}
			getLog().debugStream() << "[" << thr_self() <<
				"]\taddCorrectRouteNotMatch2(" <<
				s.value() << "): " << *data;
			routeMan->addRoute(*route);
			routeReg->putRoute(*data);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addIncorrectRoute(const SmeSystemId& smeSystemId,
	const RouteInfo& existingRoute, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			RouteInfo route;
			route.priority = rand2(RoutePriorityMin, RoutePriorityMax);
			route.billing = rand0(1);
			route.paid = rand0(1);
			route.archived = rand0(1);
			route.routeId = rand0(INT_MAX);
			route.smeSystemId = smeSystemId;
			SmsUtil::setupRandomCorrectAddress(&route.source);
			SmsUtil::setupRandomCorrectAddress(&route.dest);
			switch(s.value())
			{
				case 1: //дублирующий routeId
					//route.routeId = existingRoute.routeId;
					//break;
				case 2: //несуществующий smeSystemId
					{
						auto_ptr<char> tmp = rand_char(rand1(MAX_SYSTEM_ID_LENGTH));
						route.smeSystemId = tmp.get();
					}
					break;
				default:
					throw s;
			}
			routeMan->addRoute(route);
			res->addFailure(101);
		}
		catch(...)
		{
			//Ok
		}
	}
	debug(res);
	return res;
}

void RouteManagerTestCases::printLookupResult(const Address& origAddr,
	const Address& destAddr, const RouteRegistry::RouteList& routeList,
	bool found, const SmeProxy* proxy)
{
	ostringstream os;
	os << "lookupRoute(): ids = <";
	for (int i = 0; i < routeList.size(); i++)
	{
		if (i > 0)
		{
			os  << ',';
		}
		if (routeList[i]->proxy)
		{
			os << routeList[i]->proxy->getUniqueId();
		}
		else
		{
			os << "NULL";
		}
	}
	if (proxy)
	{
		os << ">, res = " << proxy->getUniqueId();
	}
	else if (found)
	{
		os << ">, res = <NULL>";
	}
	else
	{
		os << ">, res = <>";
	}
	os << ", origAddr = " << origAddr;
	os << ", destAddr = " << destAddr;
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
	//getLog().debugStream() << ss.str();
}

TCResult* RouteManagerTestCases::lookupRoute(const Address& origAddr,
	const Address& destAddr)
{
	TCResult* res = new TCResult(TC_LOOKUP_ROUTE);
	try
	{
		SmeProxy* proxy = NULL;
		bool found = routeMan->lookup(origAddr, destAddr, proxy);
		const RouteRegistry::RouteList routeList =
			routeReg->lookup(origAddr, destAddr);
		printLookupResult(origAddr, destAddr, routeList, found, proxy);
		if (found)
		{
			if (proxy)
			{
				uint32_t id = proxy->getUniqueId();
				found = false;
				for (int i = 0; i < routeList.size(); i++)
				{
					if (routeList[i]->proxy &&
						routeList[i]->proxy->getUniqueId() == id)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					res->addFailure(101);
				}
			}
			//маршрут есть, но proxy нет
			else if (!routeList.size())
			{
				res->addFailure(102);
			}
		}
		else if (routeList.size())
		{
			res->addFailure(103);
		}
	}
	catch(...)
	{
		error();
		printLookupResult(origAddr, destAddr, RouteRegistry::RouteList(), false, NULL);
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::iterateRoutes()
{
	TCResult* res = new TCResult(TC_ITERATE_ROUTES);
	try
	{
		RouteIterator* iter = routeMan->iterator();
		int foundRoutes = 0;
		int extraRoutes = 0;
		typedef map<int, int> MismatchMap;
		MismatchMap mismatch;
		while(iter->next())
		{
			//SmeProxy* proxy = iter->getSmeProxy();
			//int index = iter->getIndex();
			RouteInfo route = iter->getRouteInfo();
			const RouteInfo* correctRoute = routeReg->getRoute(route.routeId);
			if (!correctRoute)
			{
				extraRoutes++;
			}
			else
			{
				foundRoutes++;
				vector<int> tmp = RouteUtil::compareRoutes(*correctRoute, route);
				for (int i = 0; i < tmp.size(); i++)
				{
					mismatch[tmp[i]]++;
				}
			}
		}
		delete iter;
		//итератор вернул лишние маршруты
		if (extraRoutes)
		{
			res->addFailure(101);
		}
		//итератор пропустил некоторые маршруты
		if (foundRoutes != routeReg->size())
		{
			res->addFailure(102);
		}
		//перечислить отличия в маршрутах
		for (MismatchMap::iterator it = mismatch.begin(); it != mismatch.end(); it++)
		{
			if (it->second)
			{
				res->addFailure(it->first);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

}
}
}

