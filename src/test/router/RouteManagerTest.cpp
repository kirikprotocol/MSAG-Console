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
using smsc::test::sms::SmsUtil;

static Category& log = Logger::getCategory("RouteManagerTest");
SmeManagerTestCases tcSme;
RouteManagerTestCases tcRoute;
RouteRegistry routeReg;

void addRouteNotMatch(const Address& origAddr, const Address& destAddr,
	int smeNum, bool needProxy, int routeNum)
{
	SmeInfo sme;
	SmeProxy* proxy = NULL;
	cout << *tcSme.addCorrectSme(&sme, smeNum) << endl;
	if (needProxy)
	{
		cout << *tcSme.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;
	}
	TestRouteData routeData(origAddr, destAddr, proxy);
	if (routeNum < 1000)
	{
		cout << *tcRoute.addCorrectRouteNotMatch(sme.systemId, &routeData, routeNum) << endl;
	}
	else
	{
		cout << *tcRoute.addCorrectRouteNotMatch2(sme.systemId, &routeData, routeNum) << endl;
	}
	routeReg.putRoute(routeData);
}

void addRouteMatch(const Address& origAddr, const Address& destAddr,
	int smeNum, bool needProxy, int routeNum)
{
	SmeInfo sme;
	SmeProxy* proxy = NULL;
	cout << *tcSme.addCorrectSme(&sme, smeNum) << endl;
	if (needProxy)
	{
		cout << *tcSme.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;
	}
	TestRouteData routeData(origAddr, destAddr, proxy);
	cout << *tcRoute.addCorrectRouteMatch(sme.systemId, &routeData, routeNum) << endl;
	routeReg.putRoute(routeData);
}

void executeTest()
{
/*
addCorrectSme(10)->
registerCorrectSmeProxy(1)->
addCorrectRouteNotMatch(25)->

addCorrectSme(6)->
registerCorrectSmeProxy(1)->
addCorrectRouteMatch(18)->

addCorrectSme(5)->
registerCorrectSmeProxy(1)->
addCorrectRouteMatch(1)->
lookupRoute(1){101}
*/
	Address origAddr, destAddr;
	SmsUtil::setupRandomCorrectAddress(&origAddr);
	SmsUtil::setupRandomCorrectAddress(&destAddr);

	addRouteNotMatch(origAddr, destAddr, 10, true, 25);
	addRouteMatch(origAddr, destAddr, 6, true, 18);
	addRouteMatch(origAddr, destAddr, 5, true, 1);
	cout << *tcRoute.lookupRoute(routeReg, origAddr, destAddr) << endl;

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
		executeTest();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

