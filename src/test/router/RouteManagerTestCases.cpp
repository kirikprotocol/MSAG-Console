#include "RouteManagerTestCases.hpp"
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
using smsc::test::sms::operator<<;
using smsc::test::core::operator<<;
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

void RouteManagerTestCases::debugRoute(const char* tc, const RouteInfo* route)
{
	ostringstream os;
	os << *route;
	getLog().debug("[%d]\t%s: %s", thr_self(), tc, os.str().c_str());
	__trace2__("%s: %s", tc, os.str().c_str());
}

void RouteManagerTestCases::commit()
{
	routeMan->commit();
}

void RouteManagerTestCases::setupRandomAddressMatch(Address& addr, int num)
{
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	switch(num)
	{
		case 1: //адрес без знаков подстановки (совпадает)
			break;
		case 2: //адрес с одним или всеми '?' в конце
			memset(addrVal + addrLen - len, '?', len);
			addr.setValue(addrLen, addrVal);
			break;
		case 3: //адрес целиком из '?'
			memset(addrVal, '?', addrLen);
			addr.setValue(addrLen, addrVal);
			break;
		/*
		case 4: //адрес со '*' в конце
			memset(addrVal + addrLen - len, '*', 1);
			addr.setValue(addrLen - len + 1, addrVal);
			break;
		case 5: //адрес из '*'
			addrVal[0] = '*';
			addr.setValue(1, addrVal);
			break;
		case 6: //адрес с лишней '*'
			if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
			{
				addrVal[addrLen] = '*';
				addr.setValue(addrLen + 1, addrVal);
			}
			break;
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

TCResult* RouteManagerTestCases::addCorrectRouteMatch(RouteInfo* route,
	SmeProxy* proxy, int num)
{
	int numMatch1 = 3; int numMatch2 = 3;
	TCSelector s(num, numMatch1 * numMatch2);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(route);
			setupRandomAddressMatch(route->source, s.value1(numMatch1));
			setupRandomAddressMatch(route->dest, s.value2(numMatch1));
			if (routeReg->putRoute(*route, proxy))
			{
				char tc[64];
				sprintf(tc, "addCorrectRouteMatch(%d,%d)",
					s.value1(numMatch1), s.value2(numMatch1));
				debugRoute(tc, route);
				routeMan->addRoute(*route);
			}
		}
		catch(...)
		{
			error();
			res->addFailure(1000);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRouteNotMatch(RouteInfo* route,
	SmeProxy* proxy, int num)
{
	int numMatch = 3; int numNotMatch = 4; int numType = 2;
	TCSelector s(num, numMatch * numNotMatch * numType);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_NOT_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			char tc[64];
			RouteUtil::setupRandomCorrectRouteInfo(route);
			switch (s.value3(numMatch, numNotMatch))
			{
				case 1: //отличается origAddr
					setupRandomAddressNotMatch(route->source,
						s.value1(numNotMatch, numMatch));
					setupRandomAddressMatch(route->dest,
						s.value2(numNotMatch, numMatch));
					sprintf(tc, "addCorrectRouteNotMatch(%d,%d,%d)",
						s.value1(numNotMatch, numMatch),
						s.value2(numNotMatch, numMatch),
						s.value3(numMatch, numNotMatch));
					break;
				case 2: //отличается destAddr
					setupRandomAddressMatch(route->source,
						s.value1(numMatch, numNotMatch));
					setupRandomAddressNotMatch(route->dest,
						s.value2(numMatch, numNotMatch));
					sprintf(tc, "addCorrectRouteNotMatch(%d,%d,%d)",
						s.value1(numMatch, numNotMatch),
						s.value2(numMatch, numNotMatch),
						s.value3(numMatch, numNotMatch));
					break;
				default:
					throw s;
			}
			if (routeReg->putRoute(*route, proxy))
			{
				debugRoute(tc, route);
				routeMan->addRoute(*route);
			}
		}
		catch(...)
		{
			error();
			res->addFailure(1000);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRouteNotMatch2(RouteInfo* route,
	SmeProxy* proxy, int num)
{
	TCSelector s(num, 4, 1000);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_NOT_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(route);
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
			if (routeReg->putRoute(*route, proxy))
			{
				char tc[64];
				sprintf(tc, "addCorrectRouteNotMatch2(%d)", s.value());
				debugRoute(tc, route);
				routeMan->addRoute(*route);
			}
		}
		catch(...)
		{
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addIncorrectRoute(
	const RouteInfo& existingRoute, int num)
{
	TCSelector s(num, 1);
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			RouteInfo route;
			RouteUtil::setupRandomCorrectRouteInfo(&route);
			route.smeSystemId = existingRoute.smeSystemId;
			SmsUtil::setupRandomCorrectAddress(&route.source);
			SmsUtil::setupRandomCorrectAddress(&route.dest);
			switch(s.value())
			{
				/*
				case 1: //дублирующий routeId
					route.routeId = existingRoute.routeId;
					break;
				*/
				case 1: //несуществующий smeSystemId
					{
						auto_ptr<char> tmp = rand_char(rand1(MAX_SYSTEM_ID_LENGTH));
						route.smeSystemId = tmp.get();
					}
					break;
				default:
					throw s;
			}
			char tc[64];
			sprintf(tc, "addIncorrectRoute(%d)", s.value());
			debugRoute(tc, &route);
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
	const Address& destAddr, const RouteHolder* routeHolder,
	bool found, const SmeProxy* proxy)
{
	ostringstream os;
	os << "lookupRoute(): routeReg.lookup() = ";
	if (routeHolder && routeHolder->proxy)
	{
		os << routeHolder->proxy->getUniqueId();
	}
	else if (routeHolder)
	{
		os << "true";
	}
	else
	{
		os << "NULL";
	}
	os << ", routeMan.lookup() = ";
	if (found && proxy)
	{
		os << proxy->getUniqueId();
	}
	else if (found)
	{
		os << "true";
	}
	else
	{
		os << "NULL";
	}
	os << ", origAddr = " << origAddr;
	os << ", destAddr = " << destAddr;
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
	__trace2__("%s", os.str().c_str());
}

TCResult* RouteManagerTestCases::lookupRoute(const Address& origAddr,
	const Address& destAddr)
{
	TCResult* res = new TCResult(TC_LOOKUP_ROUTE);
	try
	{
		SmeProxy* proxy = NULL;
		__trace__("RouteManagerTestCases::lookupRoute()");
		bool found = routeMan->lookup(origAddr, destAddr, proxy);
		const RouteHolder* routeHolder = routeReg->lookup(origAddr, destAddr);
		printLookupResult(origAddr, destAddr, routeHolder, found, proxy);
		if (found && routeHolder)
		{
			if (proxy && routeHolder->proxy)
			{
				if (proxy->getUniqueId() != routeHolder->proxy->getUniqueId())
				{
					res->addFailure(101);
				}
			}
			else if (proxy && !routeHolder->proxy)
			{
				res->addFailure(102);
			}
			else if (!proxy && routeHolder->proxy)
			{
				res->addFailure(103);
			}
		}
		else if (found && !routeHolder)
		{
			res->addFailure(104);
		}
		else if (!found && routeHolder)
		{
			res->addFailure(105);
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

TCResult* RouteManagerTestCases::iterateRoutes()
{
	TCResult* res = new TCResult(TC_ITERATE_ROUTES);
	res->addFailure(100);
	debug(res);
	return res;
}

}
}
}

