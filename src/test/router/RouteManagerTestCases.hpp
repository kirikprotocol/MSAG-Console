#ifndef TEST_ROUTER_ROUTE_MANAGER_TEST_CASES
#define TEST_ROUTER_ROUTE_MANAGER_TEST_CASES

#include "sms/sms.h"
#include "router/route_manager.h"
#include "router/route_types.h"
#include "smeman/smetypes.h"
#include "test/util/BaseTestCases.hpp"
#include "test/util/Util.hpp"
#include "test/core/RouteUtil.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace router {

using std::vector;
using std::ostream;
using log4cpp::Category;
using smsc::sms::Address;
using smsc::router::RouteInfo;
using smsc::router::RouteManager;
using smsc::smeman::SmeSystemId;
using smsc::test::core::TestRouteData;
using smsc::test::util::BaseTestCases;
using smsc::test::util::TCResult;

//implemented
const char* const TC_ADD_CORRECT_ROUTE = "addCorrectRoute";
const char* const TC_ADD_INCORRECT_ROUTE = "addIncorrectRoute";
const char* const TC_LOOKUP_ROUTE = "lookupRoute";
const char* const TC_ITERATE_ROUTES = "iterateRoutes";

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
		const vector<TestRouteData*>& routes);

	/*
	 * ������������ �� ������ ���������.
	 */
	TCResult* iterateRoutes(const vector<TestRouteData*>& routes);

protected:
	virtual Category& getLog();

private:
	RouteManager* routeMan;

	void setupRandomCorrectRouteInfo(const SmeSystemId& smeSystemId,
		RouteInfo* info);
	vector<int> compareRoutes(const RouteInfo& route1, const RouteInfo& route2);
	void debugRoute(RouteInfo& route);
};

}
}
}

#endif /* TEST_ROUTER_ROUTE_MANAGER_TEST_CASES */

