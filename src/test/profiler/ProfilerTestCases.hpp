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
 * ���� ����� �������� test cases ����������� ��� ������������ profiler
 * @author bryz
 */
class ProfilerTestCases : BaseTestCases
{
public:
	ProfilerTestCases(Profiler* profiler, ProfileRegistry* profileReg,
		CheckList* chkList);

	virtual ~ProfilerTestCases() {}

	/**
	 * �������� ������ ������� ���������������� �������� ������
	 * �� ��������� ������.
	 */
	void createProfileMatch(Address& addr, int num);

	/**
	 * �������� ������ ������� ������������������ �������� ������
	 * �� ��������� ������.
	 */
	void createProfileNotMatch(Address& addr, int num);

	/**
	 * ���������� ��� ������������� �������.
	 */
	void updateProfile(const Address& addr);

	/**
	 * ����� ������� ��� ��������� ������.
	 */
	void lookup(const Address& addr);

	/**
	 * ������ � �������� ����� submit_sm pdu.
	 */
	void putCommand(const Address& addr, int num);

protected:
	virtual Category& getLog();

private:
	Profiler* profiler;
	ProfileRegistry* profileReg;
	CheckList* chkList;

	void fillAddressWithQuestionMarks(Address& addr, int len);
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_TEST_CASES */

