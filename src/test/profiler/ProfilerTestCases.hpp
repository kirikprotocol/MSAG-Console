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
using smsc::smeman::SmscCommand;
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
	 * ������ � ���������� �������� ����� deliver_sm pdu.
	 */
	void putCommand(const Address& addr, int num);

	/**
	 * ��������� ������ ����������� �� ��������� ��������.
	 */
	void onCommand();

protected:
	virtual Category& getLog();

private:
	Profiler* profiler;
	ProfileRegistry* profileReg;
	CheckList* chkList;

	void fillAddressWithQuestionMarks(Address& addr, int len);
	bool updateProfile(const char* tc, int num, const Address& addr,
		const Profile& profile, bool create);
	void onSubmit(SmscCommand& cmd);
	void onDeliveryResp(SmscCommand& cmd);
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_TEST_CASES */

