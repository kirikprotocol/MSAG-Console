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
using smsc::test::core::RouteRegistry;
using smsc::test::core::RouteHolder;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::util::TestCase;

/**
 * Этот класс содержит test cases необходимые для тестирования подсистемы
 * route manager.
 * 
 * @author bryz
 */
class RouteManagerTestCases : BaseTestCases
{
public:
	RouteManagerTestCases(RouteManager* routeMan, RouteRegistry* routeReg,
		CheckList* chkList);

	virtual ~RouteManagerTestCases() {}

	void commit();
	
	/**
	 * Добавление корректного рабочего маршрута.
	 */
	void addCorrectRouteMatch(RouteInfo* route, SmeProxy* proxy, int num);

	/**
	 * Добавление корректного нерабочего маршрута.
	 */
	void addCorrectRouteNotMatch(RouteInfo* route, SmeProxy* proxy, int num);

	/**
	 * Добавление корректного маршрута с неправильными (непроверяемыми)
	 * значениями.
	 */
	void addCorrectRouteNotMatch2(RouteInfo* route, SmeProxy* proxy, int num);

	/**
	 * Добавление некорректного маршрута.
	 */
	void addIncorrectRoute(const RouteInfo& existingRoute, int num);

	/**
	 * Поиск маршрута.
	 */
	void lookupRoute(const Address& origAddr, const Address& destAddr);

	/*
	 * Итерирование по списку маршрутов.
	 */
	//void iterateRoutes();

protected:
	virtual Category& getLog();

private:
	RouteManager* routeMan;
	RouteRegistry* routeReg;
	CheckList* chkList;

	void debugRoute(const char* tc, const RouteInfo* route);
	void addRoute(const char* tc, int num, const RouteInfo* route, SmeProxy* proxy);
	TestCase* setupRandomAddressMatch(Address& addr, const char type, int num);
	TestCase* setupRandomAddressNotMatch(Address& addr, const char type, int num);
	void printLookupResult(const Address& origAddr, const Address& destAddr,
		const RouteHolder* routeHolder, bool found, const SmeProxy* proxy);
};

}
}
}

#endif /* TEST_ROUTER_ROUTE_MANAGER_TEST_CASES */

