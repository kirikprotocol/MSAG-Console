#ifndef TEST_ADMIN_ADMIN_ALIAS_TEST_CASES
#define TEST_ADMIN_ADMIN_ALIAS_TEST_CASES

#include "AdminBaseTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using log4cpp::Category;
using smsc::test::util::CheckList;

/**
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
 * admin console ��� ������ � ��������.
 * @author bryz
 */
class AdminAliasTestCases : public AdminBaseTestCases
{
public:
	AdminAliasTestCases(AdminFixture* fixture)
		: AdminBaseTestCases(fixture) {}

	virtual ~AdminAliasTestCases() {}

	void correctCommands();
	void incorrectCommands();

protected:
	virtual Category& getLog();
};

}
}
}

#endif /* TEST_ADMIN_ADMIN_ALIAS_TEST_CASES */

