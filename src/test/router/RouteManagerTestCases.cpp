#include "RouteManagerTestCases.hpp"
#include "test/smeman/SmeManagerUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/core/RouteUtil.hpp"
#include "test/util/Util.hpp"
#include "util/debug.h"
#include <algorithm>
#include <sstream>
#include <sys/types.h>

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
	RouteRegistry* _routeReg, CheckList* _chkList)
	: routeMan(_routeMan), routeReg(_routeReg), chkList(_chkList)
{
	//__require__(routeMan);
	//__require__(routeReg);
	//__require__(chkList);
}

Category& RouteManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("RouteManagerTestCases");
	return log;
}

void RouteManagerTestCases::debugRoute(const char* tc, const RouteInfo* route)
{
	__trace2__("%s: %s", tc, str(*route).c_str());
}

void RouteManagerTestCases::commit()
{
	routeMan->commit();
}

#define __tc_cond__(id1, id2) \
	if (type == 's') { \
		__tc__(id1 ".source" id2); \
	} else if ( type == 'd') { \
		__tc__(id1 ".dest" id2); \
	}
	
TestCase* RouteManagerTestCases::setupRandomAddressMatch(Address& addr,
	const char type, int num)
{
	__decl_tc__;
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	switch(num)
	{
		case 1: //адрес без знаков подстановки (совпадает)
			__tc_cond__("addCorrectRoute", "MatchNoSubstSymbols");
			break;
		case 2: //адрес с одним или всеми '?' в конце
			__tc_cond__("addCorrectRoute", "MatchWithQuestionMarks");
			memset(addrVal + addrLen - len, '?', len);
			addr.setValue(addrLen, addrVal);
			break;
		case 3: //адрес целиком из '?'
			__tc_cond__("addCorrectRoute", "MatchEntirelyQuestionMarks");
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
			__unreachable__("Invalid num");
	}
	return tc;
}

TestCase* RouteManagerTestCases::setupRandomAddressNotMatch(Address& addr,
	const char type, int num)
{
	__decl_tc__;
	__tc_cond__("addCorrectRoute", "MatchNoSubstSymbols");
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	switch(num)
	{
		case 1: //адрес с лишним '?'
			if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
			{
				__tc_cond__("addCorrectRoute", "NotMatchValueLength");
				memset(addrVal + addrLen - len, '?', len + 1);
				addr.setValue(addrLen + 1, addrVal);
				break;
			}
			//break;
		case 2: //адрес с недостающим '?'
			if (addrLen > 1)
			{
				__tc_cond__("addCorrectRoute", "NotMatchValueLength");
				memset(addrVal + addrLen - len, '?', len - 1);
				addr.setValue(addrLen - 1, addrVal);
				break;
			}
			//break;
		case 3: //отличающийся адрес
			if (next_permutation(addrVal, addrVal + addrLen))
			{
				__tc_cond__("addCorrectRoute", "NotMatchValue");
				addr.setValue(addrLen, addrVal);
			}
			break;
		case 4: //адрес с несовпадающим typeOfNumber
			__tc_cond__("addCorrectRoute", "NotMatchType");
			addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
			break;
		case 5: //адрес с несовпадающим numberingPlan
			__tc_cond__("addCorrectRoute", "NotMatchPlan");
			addr.setNumberingPlan(addr.getNumberingPlan() + 1);
			break;
		default:
			__unreachable__("Invalid num");
	}
	return tc;
}

void RouteManagerTestCases::addRoute(const char* tc, int num,
	const RouteInfo* route, SmeProxy* proxy)
{
	if (routeReg)
	{
		if (routeReg->putRoute(*route, proxy))
		{
			debugRoute(tc, route);
			if (routeMan)
			{
				routeMan->addRoute(*route);
			}
		}
	}
	else if (routeMan)
	{
		debugRoute(tc, route);
		routeMan->addRoute(*route);
	}
}

void RouteManagerTestCases::addCorrectRouteMatch(RouteInfo* route,
	SmeProxy* proxy, int num)
{
	int numMatch1 = 3; int numMatch2 = 3;
	TCSelector s(num, numMatch1 * numMatch2);
	__decl_tc12__;
	for (; s.check(); s++)
	{
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(route);
			tc1 = setupRandomAddressMatch(route->source, 's', s.value1(numMatch1));
			tc2 = setupRandomAddressMatch(route->dest, 'd', s.value2(numMatch1));
			char tcId[64];
			sprintf(tcId, "addCorrectRouteMatch(%d,%d)",
				s.value1(numMatch1), s.value2(numMatch1));
			addRoute(tcId, s.value(), route, proxy);
			__tc12_ok__;
		}
		catch(...)
		{
			__tc12_fail__(1000);
			error();
		}
	}
}

void RouteManagerTestCases::addCorrectRouteNotMatch(RouteInfo* route,
	SmeProxy* proxy, int num)
{
	int numMatch = 3; int numNotMatch = 5; int numType = 2;
	TCSelector s(num, numMatch * numNotMatch * numType);
	__decl_tc12__;
	for (; s.check(); s++)
	{
		try
		{
			char tcId[64];
			RouteUtil::setupRandomCorrectRouteInfo(route);
			switch (s.value3(numMatch, numNotMatch))
			{
				case 1: //отличается origAddr
					tc1 = setupRandomAddressNotMatch(route->source, 's',
						s.value1(numNotMatch, numMatch));
					tc2 = setupRandomAddressMatch(route->dest, 'd',
						s.value2(numNotMatch, numMatch));
					sprintf(tcId, "addCorrectRouteNotMatch(%d,%d,%d)",
						s.value1(numNotMatch, numMatch),
						s.value2(numNotMatch, numMatch),
						s.value3(numMatch, numNotMatch));
					break;
				case 2: //отличается destAddr
					tc1 = setupRandomAddressMatch(route->source, 's',
						s.value1(numMatch, numNotMatch));
					tc2 = setupRandomAddressNotMatch(route->dest, 'd',
						s.value2(numMatch, numNotMatch));
					sprintf(tcId, "addCorrectRouteNotMatch(%d,%d,%d)",
						s.value1(numMatch, numNotMatch),
						s.value2(numMatch, numNotMatch),
						s.value3(numMatch, numNotMatch));
					break;
				default:
					__unreachable__("Invalid num");
			}
			addRoute(tcId, s.value(), route, proxy);
			__tc12_ok__;
		}
		catch(...)
		{
			__tc12_fail__(1000);
			error();
		}
	}
}

void RouteManagerTestCases::addCorrectRouteNotMatch2(RouteInfo* route,
	SmeProxy* proxy, int num)
{
	TCSelector s(num, 4, 1000);
	__decl_tc__;
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
					__tc__("addIncorrectRoute.invalidType");
					route->source.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 1002: //origAddr.numberingPlan вне диапазона
					__tc__("addIncorrectRoute.invalidPlan");
					route->source.setNumberingPlan(rand2(0x13, 0xff));
					break;
				case 1003: //destAddr.typeOfNumber вне диапазона
					__tc__("addIncorrectRoute.invalidType");
					route->dest.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 1004: //destAddr.numberingPlan вне диапазона
					__tc__("addIncorrectRoute.invalidPlan");
					route->dest.setNumberingPlan(rand2(0x13, 0xff));
					break;
				default:
					throw s;
			}
			char tcId[64];
			sprintf(tcId, "addCorrectRouteNotMatch2(%d)", s.value());
			addRoute(tcId, s.value(), route, proxy);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void RouteManagerTestCases::addIncorrectRoute(
	const RouteInfo& existingRoute, int num)
{
	TCSelector s(num, 1);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			RouteInfo route;
			RouteUtil::setupRandomCorrectRouteInfo(&route);
			route.enabling = rand0(2);
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
						__tc__("addIncorrectRoute.invalidSmeId");
						auto_ptr<char> tmp = rand_char(rand1(MAX_SYSTEM_ID_LENGTH));
						route.smeSystemId = tmp.get();
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			char tcId[64];
			sprintf(tcId, "addIncorrectRoute(%d)", s.value());
			debugRoute(tcId, &route);
			if (routeMan)
			{
				routeMan->addRoute(route);
				__tc_fail__(101);
			}
		}
		catch(...)
		{
			__tc_ok__;
		}
	}
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
	__trace2__("%s", os.str().c_str());
}

void RouteManagerTestCases::lookupRoute(const Address& origAddr,
	const Address& destAddr)
{
	__require__(routeReg && routeMan);
	__decl_tc__;
	__tc__("lookupRoute");
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
					__tc_fail__(101);
				}
			}
			else if (proxy && !routeHolder->proxy)
			{
				__tc_fail__(102);
			}
			else if (!proxy && routeHolder->proxy)
			{
				__tc_fail__(103);
			}
		}
		else if (found && !routeHolder)
		{
			__tc_fail__(104);
		}
		else if (!found && routeHolder)
		{
			__tc_fail__(105);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

/*
void RouteManagerTestCases::iterateRoutes()
{
	void res = new TCResult(TC_ITERATE_ROUTES);
	__tc_fail__(100);
	debug(res);
	return res;
}
*/

}
}
}

