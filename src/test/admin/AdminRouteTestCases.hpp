#ifndef TEST_ADMIN_ADMIN_ROUTE_TEST_CASES
#define TEST_ADMIN_ADMIN_ROUTE_TEST_CASES

#include "AdminBaseTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::test::util::CheckList;

/**
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
 * admin console ��� ������ � ���������� � ����������.
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
	virtual Logger getLog();
	const char* getSubjectName(int i);
};

}
}
}

#endif /* TEST_ADMIN_ADMIN_ROUTE_TEST_CASES */

