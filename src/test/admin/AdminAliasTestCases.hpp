#ifndef TEST_ADMIN_ADMIN_ALIAS_TEST_CASES
#define TEST_ADMIN_ADMIN_ALIAS_TEST_CASES

#include "AdminBaseTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using log4cpp::Category;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит test cases необходимые для тестирования подсистемы
 * admin console при работе с алиасами.
 * @author bryz
 */
class AdminAliasTestCases : public AdminBaseTestCases
{
	bool hasRights;

public:
	AdminAliasTestCases(AdminFixture* fixture, bool _hasRights)
		: AdminBaseTestCases(fixture), hasRights(_hasRights) {}

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

