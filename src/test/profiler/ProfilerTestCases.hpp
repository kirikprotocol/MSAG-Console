#ifndef TEST_PROFILER_PROFILER_TEST_CASES
#define TEST_PROFILER_PROFILER_TEST_CASES

#include "profiler/profiler.hpp"
#include "test/util/BaseTestCases.hpp"
#include "test/core/ProfileRegistry.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace profiler {

using log4cpp::Category;
using std::vector;
using smsc::profiler::Profile;
using smsc::profiler::Profiler;
using smsc::sms::Address;
using smsc::test::util::BaseTestCases;
using smsc::test::core::ProfileRegistry;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит test cases необходимые для тестирования profiler
 * @author bryz
 */
class ProfilerTestCases : BaseTestCases
{
public:
	ProfilerTestCases(Profiler* profiler, ProfileRegistry* profileReg,
		CheckList* chkList);

	virtual ~ProfilerTestCases() {}

	/**
	 * Создание нового профиля удовлетворяющего условиям поиска
	 * по заданному адресу.
	 */
	void createProfileMatch(Address& addr, int num);

	/**
	 * Создание нового профиля неудовлетворяющего условиям поиска
	 * по заданному адресу.
	 */
	void createProfileNotMatch(Address& addr, int num);

	/**
	 * Обновление уже существующего профиля.
	 */
	void updateProfile(const Address& addr);

	/**
	 * Поиск профиля для заданного адреса.
	 */
	void lookup(const Address& addr);

	/**
	 * Работа с профилем через submit_sm pdu.
	 */
	void putCommand(Address& addr, int num);

	static void setupRandomCorrectProfile(Profile& profile);

protected:
	virtual Category& getLog();

private:
	Profiler* profiler;
	ProfileRegistry* profileReg;
	CheckList* chkList;

	vector<int> compareProfiles(const Profile& p1, const Profile& p2);
	void fillAddressWithQuestionMarks(Address& addr, int len);
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_TEST_CASES */

