#ifndef TEST_ADMIN_ADMIN_ROUTE_TEST_CASES
#define TEST_ADMIN_ADMIN_ROUTE_TEST_CASES

#include "AdminBaseTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using log4cpp::Category;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит test cases необходимые для тестирования подсистемы
 * admin console при работе с субъектами и маршрутами.
 * @author bryz
 */
class AdminRouteTestCases : public AdminBaseTestCases
{
	bool hasRights;

public:
	AdminRouteTestCases(AdminFixture* fixture, bool _hasRights)
		: AdminBaseTestCases(fixture), hasRights(_hasRights) {}

	virtual ~AdminRouteTestCases() {}

	void correctSubjectCommands();
	void incorrectSubjectCommands();

	void correctRouteCommands();
	void incorrectRouteCommands();

protected:
	virtual Category& getLog();
	const char* getSubjectName(int i);
};

}
}
}

#endif /* TEST_ADMIN_ADMIN_ROUTE_TEST_CASES */

