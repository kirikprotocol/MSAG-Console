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
		//putCommand
		__reg_tc__("putCommand", "���������� ���������� �������� ����� deliver_sm pdu");
		__reg_tc__("putCommand.reportNoneMixedCase",
			"���������� �� ����������� � �������� (����� ������� � ��������� ��������)");
		__reg_tc__("putCommand.reportNoneSpaces",
			"���������� �� ����������� � �������� (����� ������� � ������� ���������)");
		__reg_tc__("putCommand.reportFullMixedCase",
			"���������� ����� ����������� � �������� (����� ������� � ��������� ��������)");
		__reg_tc__("putCommand.reportFullSpaces",
			"���������� ����� ����������� � �������� (����� ������� � ������� ���������)");
		__reg_tc__("putCommand.ucs2CodepageMixedCase",
			"���������� ����� ������ ��������� �� ������� ����� (��������� ucs2, ����� ������� � ��������� ��������)");
		__reg_tc__("putCommand.ucs2CodepageSpaces",
			"���������� ����� ������ ��������� �� ������� ����� (��������� ucs2, ����� ������� � ������� ���������)");
		__reg_tc__("putCommand.defaultCodepageMixedCase",
			"���������� �� ������ ������ ��������� �� ������� ����� (����� ������� � ��������� ��������)");
		__reg_tc__("putCommand.defaultCodepageSpaces",
			"���������� �� ������ ������ ��������� �� ������� ����� (����� ������� � ������� ���������)");
		__reg_tc__("putCommand.incorrectText",
			"������������ ����� �������");
		//getCommand
		__reg_tc__("getCommand", "��������� ������� �� ��������� ��������");
		__reg_tc__("getCommand.submit",
			"��������� submit_sm pdu �� �������� ���������� �������� ���������� �������");
		__reg_tc__("getCommand.submit.checkDialogId",
			"���������� �������� dialogId");
		__reg_tc__("getCommand.submit.checkFields",
			"���������� �������� ����� sms (������� ����� ���������)");
		__reg_tc__("getCommand.deliverResp",
			"��������� deliver_sm_resp pdu � ����� �� submit_sm");
	}
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_CHECK_LIST */

