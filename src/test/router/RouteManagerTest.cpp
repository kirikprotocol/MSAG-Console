#include "RouteManagerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "util/Logger.h"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using namespace smsc::test::router; //constants, TestRouteData
using log4cpp::Category;
using smsc::sms::Address;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::sms::operator<<;
using smsc::test::sms::SmsUtil;

static Category& log = Logger::getCategory("RouteManagerTest");

class RouteManagerTest
{
	SmeManagerTestCases tcSme;
	RouteManagerTestCases tcRoute;

public:
	RouteManagerTest(SmeManager* smeMan, RouteManager* routeMan,
		RouteRegistry* routeReg, SmeRegistry* smeReg)
		: tcSme(smeMan, smeReg, NULL), tcRoute(routeMan, routeReg, NULL) {}

	void addRouteNotMatch(const Address& origAddr, const Address& destAddr,
		int smeNum, bool needProxy, int routeNum);
	void addRouteMatch(const Address& origAddr, const Address& destAddr,
		int smeNum, bool needProxy, int routeNum);
	void execute();
};

void RouteManagerTest::addRouteNotMatch(const Address& origAddr,
	const Address& destAddr, int smeNum, bool needProxy, int routeNum)
{
	SmeInfo sme;
	SmeProxy* proxy = NULL;
	Address smeAddr;
	tcSme.addCorrectSme(&smeAddr, &sme, smeNum);
	if (needProxy)
	{
		tcSme.registerCorrectSmeProxy(sme.systemId, &proxy);
	}
	RouteInfo route;
	route.smeSystemId = sme.systemId;
	route.source = origAddr;
	route.dest = destAddr;
	if (routeNum < 1000)
	{
		tcRoute.addCorrectRouteNotMatch(&route, proxy, routeNum);
	}
	else
	{
		tcRoute.addCorrectRouteNotMatch2(&route, proxy, routeNum);
	}
}

void RouteManagerTest::addRouteMatch(const Address& origAddr, const Address& destAddr,
	int smeNum, bool needProxy, int routeNum)
{
	SmeInfo sme;
	SmeProxy* proxy = NULL;
	Address smeAddr;
	tcSme.addCorrectSme(&smeAddr, &sme, smeNum);
	if (needProxy)
	{
		tcSme.registerCorrectSmeProxy(sme.systemId, &proxy);
	}
	RouteInfo route;
	route.smeSystemId = sme.systemId;
	route.source = origAddr;
	route.dest = destAddr;
	route.enabling = rand0(2);
	tcRoute.addCorrectRouteMatch(&route, proxy, routeNum);
}

void RouteManagerTest::execute()
{
/*
*/
	Address origAddr, destAddr;
	SmsUtil::setupRandomCorrectAddress(&origAddr);
	SmsUtil::setupRandomCorrectAddress(&destAddr);
	cout << "origAddr = " << origAddr << endl;
	cout << "destAddr = " << destAddr << endl;

	addRouteNotMatch(origAddr, destAddr, 1, true, 2);
	addRouteMatch(origAddr, destAddr, 1, true, 1);
	addRouteMatch(origAddr, destAddr, 1, true, 7);
	tcRoute.commit();
	tcRoute.lookupRoute(origAddr, destAddr);
/*
	tcRoute.addCorrectRouteMatch(sme.systemId, &routeData, 1);
	tcRoute.addCorrectRouteNotMatch(sme.systemId, &routeData, 1);
	tcRoute.addCorrectRouteNotMatch2(sme.systemId, &routeData, 1);
	tcRoute.addIncorrectRoute(sme.systemId, routeInfo, 1);
	tcRoute.lookupRoute(routeReg, origAddr, destAddr);
	tcRoute.iterateRoutes(routeReg);
*/

}

int main(int argc, char* argv[])
{
	try
	{
		//Manager::init("config.xml");
		SmeManager smeMan;
		RouteManager routeMan;
		routeMan.assign(&smeMan);
		RouteRegistry routeReg;
		SmeRegistry smeReg;
		RouteManagerTest test(&smeMan, &routeMan, &routeReg, &smeReg);
		test.execute();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

