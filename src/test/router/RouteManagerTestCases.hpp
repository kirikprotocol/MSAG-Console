#ifndef TEST_ROUTER_ROUTE_MANAGER_TEST_CASES
#define TEST_ROUTER_ROUTE_MANAGER_TEST_CASES

#include "sms/sms.h"
#include "router/route_manager.h"
#include "router/route_types.h"
#include "smeman/smetypes.h"
#include "test/util/BaseTestCases.hpp"
#include "test/util/Util.hpp"
#include "test/core/RouteUtil.hpp"
#include "test/core/RouteRegistry.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace router {

using std::vector;
using std::ostream;
using log4cpp::Category;
using smsc::sms::Address;
using smsc::router::RouteInfo;
using smsc::router::RoutePriority;
using smsc::router::RouteManager;
using smsc::smeman::SmeSystemId;
using smsc::smeman::SmeProxy;
using smsc::test::core::TestRouteData;
using smsc::test::core::RouteRegistry;
using smsc::test::util::BaseTestCases;
using smsc::test::util::TCResult;

//implemented
const char* const TC_ADD_CORRECT_ROUTE_MATCH = "addCorrectRouteMatch";
const char* const TC_ADD_CORRECT_ROUTE_NOT_MATCH = "addCorrectRouteNotMatch";
const char* const TC_ADD_INCORRECT_ROUTE = "addIncorrectRoute";
const char* const TC_LOOKUP_ROUTE = "lookupRoute";
const char* const TC_ITERATE_ROUTES = "iterateRoutes";

/**
 * Этот класс содержит test cases необходимые для тестирования подсистемы
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
	 * Добавление корректного рабочего маршрута.
	 */
	TCResult* addCorrectRouteMatch(const SmeSystemId& smeSystemId,
		TestRouteData* data, int num);

	/**
	 * Добавление корректного нерабочего маршрута.
	 */
	TCResult* addCorrectRouteNotMatch(const SmeSystemId& smeSystemId,
		TestRouteData* data, int num);

	/**
	 * Добавление корректного маршрута с неправильными (непроверяемыми)
	 * значениями.
	 */
	TCResult* addCorrectRouteNotMatch2(const SmeSystemId& smeSystemId,
		TestRouteData* data, int num);

	/**
	 * Добавление некорректного маршрута.
	 */
	TCResult* addIncorrectRoute(const SmeSystemId& smeSystemId,
		const RouteInfo& existingRoute, int num);

	/**
	 * Поиск маршрута.
	 */
	TCResult* lookupRoute(const RouteRegistry& routeReg,
		const Address& origAddr, const Address& destAddr);

	/*
	 * Итерирование по списку маршрутов.
	 */
	TCResult* iterateRoutes(const RouteRegistry& routeReg);

protected:
	virtual Category& getLog();

private:
	RouteManager* routeMan;
	float setupRandomAddressMatch(Address& addr, int num);
	void setupRandomAddressNotMatch(Address& addr, int num);
	void printLookupResult(const Address& origAddr, const Address& destAddr,
		const vector<const SmeProxy*>& ids, const SmeProxy* proxy);
};

}
}
}

#endif /* TEST_ROUTER_ROUTE_MANAGER_TEST_CASES */

