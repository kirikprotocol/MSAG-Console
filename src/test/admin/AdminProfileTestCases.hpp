#ifndef TEST_ADMIN_ADMIN_PROFILE_TEST_CASES
#define TEST_ADMIN_ADMIN_PROFILE_TEST_CASES

#include "AdminBaseTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using log4cpp::Category;
using smsc::test::util::CheckList;

static const char* noSC = "/^Failed: .*. Cause: Couldn't connect to .*, nested: Connection refused$/";

/**
 * Этот класс содержит test cases необходимые для тестирования подсистемы
 * admin console при работе с профилями.
 * @author bryz
 */
class AdminProfileTestCases : public AdminBaseTestCases
{
	bool hasRights;
	bool scRunning;

public:
	AdminProfileTestCases(AdminFixture* fixture, bool _hasRights, bool _scRunning)
		: AdminBaseTestCases(fixture), hasRights(_hasRights), scRunning(_scRunning) {}

	virtual ~AdminProfileTestCases() {}

	void correctCommands();
	void incorrectCommands();

protected:
	virtual Category& getLog();
	const char* getReport(const char* mask);
	const char* getEncoding(const char* mask);
};

}
}
}

#endif /* TEST_ADMIN_ADMIN_PROFILE_TEST_CASES */

