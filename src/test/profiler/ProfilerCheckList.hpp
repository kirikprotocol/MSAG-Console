#ifndef TEST_PROFILER_PROFILER_CHECK_LIST
#define TEST_PROFILER_PROFILER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace profiler {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class ProfilerCheckList : public CheckList
{
public:
	ProfilerCheckList()
		: CheckList("���������� ��������������� ������������ Profiler", "profiler.chk")
	{
		//createProfileMatch
		__reg_tc__("createProfileMatch",
			"�������� ������ ������� ��� �������������� ������");
		__reg_tc__("createProfileMatch.noSubstSymbols",
			"����� � �������� ������������� ������ (��� ������ �����������)");
		__reg_tc__("createProfileMatch.withQuestionMarks",
			"����� ������������� ������ � ����� ��� ����������� '?' � �����");
		__reg_tc__("createProfileMatch.entirelyQuestionMarks",
			"����� ������� �� '?', ����� ������������� ��������� ������");
		//createProfileNotMatch
		__reg_tc__("createProfileNotMatch",
			"�������� ������ ������� � ��������� ��� �������������� ������");
		__reg_tc__("createProfileNotMatch.diffType",
			"� ����� ���������� typeOfNumber");
		__reg_tc__("createProfileNotMatch.diffPlan",
			"� ����� ���������� numberingPlan");
		__reg_tc__("createProfileNotMatch.diffValue",
			"� ������ ������ ����� ���������� ���� ������, �� ����� ������ �������������");
		__reg_tc__("createProfileNotMatch.diffValueLength",
			"� ������ ������ ����� ��� ������� ������������� ��������, �� ���������� '?' ������ ������ ������������ �����");
		//other
		__reg_tc__("updateProfile", "���������� ��� ������������� �������");
		__reg_tc__("lookup", "����� ������� ��� ��������� ������");
	}
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_CHECK_LIST */

