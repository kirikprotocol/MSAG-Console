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
	
	Address addr1, addr2;
	SmsUtil::setupRandomCorrectAddress(&addr1);
	SmsUtil::setupRandomCorrectAddress(&addr2);

	SmeInfo sme;
	TestRouteData routeData(addr1, addr2);

	cout << *tcSme.addCorrectSme(&sme, 6) << endl;
	cout << *tcSme.registerCorrectSmeProxy(sme.systemId) << endl;
	cout << *tcRoute.addCorrectRoute(sme.systemId, &routeData, 1) << endl;
	//cout << routeData << endl;
	cout << *tcRoute.addIncorrectRoute(sme.systemId, *routeData.route, 1) << endl;

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

