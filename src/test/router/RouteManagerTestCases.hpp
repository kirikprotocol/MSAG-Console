#ifndef TEST_ROUTER_ROUTE_MANAGER_TEST_CASES
#define TEST_ROUTER_ROUTE_MANAGER_TEST_CASES

#include "sms/sms.h"
#include "router/route_manager.h"
#include "router/route_types.h"
#include "smeman/smetypes.h"
#include "test/util/BaseTestCases.hpp"
#include "test/util/Util.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace router {

using std::vector;
using smsc::sms::Address;
using smsc::router::RouteInfo;
using smsc::router::RouteManager;
using smsc::smeman::SmeSystemId;
using smsc::test::util::BaseTestCases;
using smsc::test::util::TCResult;

//implemented
const char* const TC_ADD_CORRECT_ROUTE = "addCorrectRoute";
const char* const TC_ADD_INCORRECT_ROUTE = "addIncorrectRoute";
const char* const TC_LOOKUP_ROUTE = "lookupRoute";
const char* const TC_ITERATE_ROUTES = "iterateRoutes";

struct TestRouteData
{
	bool match;
	float origAddrMatch;
	float destAddrMatch;
	const Address origAddr;
	const Address destAddr;
	RouteInfo* route;

	TestRouteData(const Address& _origAddr, const Address& _destAddr)
		: match(true), origAddrMatch(0.0), destAddrMatch(0.0),
		origAddr(_origAddr), destAddr(_destAddr), route(NULL) {}

	~TestRouteData()
	{
		if (route)
		{
			delete route;
		}
	}
};

/**
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
 * route manager.
 * 
 * @author bryz
 */
class RouteManagerTestCases : BaseTestCases
{
public:
	RouteManagerTestCases();

	virtual ~RouteManagerTestCases() {}

	/**
	 * ���������� ����������� ��������.
	 */
	TCResult* addCorrectRoute(const SmeSystemId& smeSystemId,
		TestRouteData* data, int num);

	/**
	 * ���������� ����������� �������� � ������������� (��������������)
	 * ����������.
	 */
	TCResult* addCorrectRoute2(const SmeSystemId& smeSystemId,
		TestRouteData* data, int num);

	/**
	 * ���������� ������������� ��������.
	 */
	TCResult* addIncorrectRoute(const SmeSystemId& smeSystemId,
		const RouteInfo& existingRoute, int num);

	/**
	 * ����� ��������.
	 */
	TCResult* lookupRoute(const Address& origAddr, const Address& destAddr,
		const vector<const TestRouteData*>& routes);

	/*
	 * ������������ �� ������ ���������.
	 */
	TCResult* iterateRoutes(const vector<const TestRouteData*>& routes);

private:
	RouteManager* routeMan;

	void setupRandomCorrectRouteInfo(const SmeSystemId& smeSystemId,
		RouteInfo* info);
	vector<int> compareRoutes(const RouteInfo& route1, const RouteInfo& route2);
};

}
}
}

#endif /* TEST_ROUTER_ROUTE_MANAGER_TEST_CASES */

