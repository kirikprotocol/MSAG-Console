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

void executeTest()
{
	SmeManagerTestCases tcSme;
	RouteManagerTestCases tcRoute;
	RouteRegistry routeReg;
/*
addCorrectSme(3)->
addCorrectRouteMatch(46)->
lookupRoute(1){104}
*/
	SmeInfo sme;
	cout << *tcSme.addCorrectSme(&sme, 3) << endl;

	SmeProxy* proxy;
	cout << *tcSme.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;

	Address origAddr, destAddr;
	SmsUtil::setupRandomCorrectAddress(&origAddr);
	SmsUtil::setupRandomCorrectAddress(&destAddr);
	TestRouteData routeData(origAddr, destAddr, proxy);
	cout << *tcRoute.addCorrectRouteMatch(sme.systemId, &routeData, 46) << endl;
	routeReg.putRoute(routeData);
	
	cout << *tcRoute.lookupRoute(routeReg, origAddr, destAddr) << endl;

/*
	cout << *tcSme.addCorrectSme(&sme, 1) << endl;
	cout << *tcSme.registerCorrectSmeProxy(sme.systemId) << endl;
	cout << *tcRoute.addCorrectRoute(sme.systemId, &routeData, 1) << endl;
	cout << *tcRoute.addCorrectRoute2(sme.systemId, &routeData, 1) << endl;
	cout << *tcRoute.addIncorrectRoute(sme.systemId, *routeData.route, 1) << endl;
	cout << *tcRoute.lookupRoute(origAddr, destAddr, routeData) << endl;
	cout << *tcRoute.iterateRoutes(routeData) << endl;
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

