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
		RouteRegistry* routeReg)
		: tcSme(smeMan), tcRoute(routeMan, routeReg) {}

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
	cout << *tcSme.addCorrectSme(&sme, smeNum) << endl;
	if (needProxy)
	{
		cout << *tcSme.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;
	}
	RouteInfo route;
	route.smeSystemId = sme.systemId;
	route.source = origAddr;
	route.dest = destAddr;
	if (routeNum < 1000)
	{
		cout << *tcRoute.addCorrectRouteNotMatch(&route, proxy, routeNum) << endl;
	}
	else
	{
		cout << *tcRoute.addCorrectRouteNotMatch2(&route, proxy, routeNum) << endl;
	}
}

void RouteManagerTest::addRouteMatch(const Address& origAddr, const Address& destAddr,
	int smeNum, bool needProxy, int routeNum)
{
	SmeInfo sme;
	SmeProxy* proxy = NULL;
	cout << *tcSme.addCorrectSme(&sme, smeNum) << endl;
	if (needProxy)
	{
		cout << *tcSme.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;
	}
	RouteInfo route;
	route.smeSystemId = sme.systemId;
	route.source = origAddr;
	route.dest = destAddr;
	cout << *tcRoute.addCorrectRouteMatch(&route, proxy, routeNum) << endl;
}

void RouteManagerTest::execute()
{
/*
addCorrectSme(11)->registerCorrectSmeProxy(1)->addCorrectRouteMatch(5)->addIncorrectRoute(1){101}
*/
	Address origAddr, destAddr;
	SmsUtil::setupRandomCorrectAddress(&origAddr);
	SmsUtil::setupRandomCorrectAddress(&destAddr);
	cout << "origAddr = " << origAddr << endl;
	cout << "destAddr = " << destAddr << endl;

	//addRouteMatch(origAddr, destAddr, 11, true, 5);

	SmeInfo sme;
	SmeProxy* proxy = NULL;
	cout << *tcSme.addCorrectSme(&sme, 11) << endl;
	cout << *tcSme.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;
	
	RouteInfo route;
	route.smeSystemId = sme.systemId;
	route.source = origAddr;
	route.dest = destAddr;
	cout << *tcRoute.addCorrectRouteNotMatch(&route, proxy, 5) << endl;
	cout << *tcRoute.addIncorrectRoute(route, 1) << endl;
	tcRoute.commit();
	cout << *tcRoute.lookupRoute(origAddr, destAddr) << endl;

/*
	cout << *tcRoute.addCorrectRouteMatch(sme.systemId, &routeData, 1) << endl;
	cout << *tcRoute.addCorrectRouteNotMatch(sme.systemId, &routeData, 1) << endl;
	cout << *tcRoute.addCorrectRouteNotMatch2(sme.systemId, &routeData, 1) << endl;
	cout << *tcRoute.addIncorrectRoute(sme.systemId, routeInfo, 1) << endl;
	cout << *tcRoute.lookupRoute(routeReg, origAddr, destAddr) << endl;
	cout << *tcRoute.iterateRoutes(routeReg) << endl;
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
		RouteManagerTest test(&smeMan, &routeMan, &routeReg);
		test.execute();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

