#ifndef TEST_SME_SMPP_PROFILER_CHECK_LIST
#define TEST_SME_SMPP_PROFILER_CHECK_LIST

#include "SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmppProfilerCheckList : public SmppProtocolCheckList
{
void allProfilerTc()
{
	//updateProfile
	__reg_tc__("updateProfile",
		"�������� smpp ��������� ��������� ��������");
	__reg_tc__("updateProfile.cmdTextDefault",
		"����� ������� � ��������� ��������� SC");
	__reg_tc__("updateProfile.cmdTextUcs2",
		"����� ������� � ��������� UCS2");
	__reg_tc__("updateProfile.reportOptions",
		"��������� ����� ����������� � ��������");
	__reg_tc__("updateProfile.reportOptions.reportNoneMixedCase",
		"���������� �� ����������� � �������� (����� ������� � ��������� ��������)");
	__reg_tc__("updateProfile.reportOptions.reportNoneSpaces",
		"���������� �� ����������� � �������� (����� ������� � ������� ���������)");
	__reg_tc__("updateProfile.reportOptions.reportFullMixedCase",
		"���������� ����� ����������� � �������� (����� ������� � ��������� ��������)");
	__reg_tc__("updateProfile.reportOptions.reportFullSpaces",
		"���������� ����� ����������� � �������� (����� ������� � ������� ���������)");
	__reg_tc__("updateProfile.dataCoding",
		"��������� ������ ������ ���������");
	__reg_tc__("updateProfile.dataCoding.ucs2CodepageMixedCase",
		"���������� ����� ������ ��������� �� ������� ����� (��������� ucs2, ����� ������� � ��������� ��������)");
	__reg_tc__("updateProfile.dataCoding.ucs2CodepageSpaces",
		"���������� ����� ������ ��������� �� ������� ����� (��������� ucs2, ����� ������� � ������� ���������)");
	__reg_tc__("updateProfile.dataCoding.defaultCodepageMixedCase",
		"���������� �� ������ ������ ��������� �� ������� ����� (����� ������� � ��������� ��������)");
	__reg_tc__("updateProfile.dataCoding.defaultCodepageSpaces",
		"���������� �� ������ ������ ��������� �� ������� ����� (����� ������� � ������� ���������)");
	__reg_tc__("updateProfile.incorrectCmdText",
		"������������ ����� �������");
	//processUpdateProfile
	__reg_tc__("processUpdateProfile",
		"��������� �� ��������� ��������");
	__reg_tc__("processUpdateProfile.checkFields",
		"���� pdu service_type, source_addr, data_coding, protocol_id, user_message_reference � �.�. ���������� (������������� ���������� ��������� �������� � ����������� ���������)");
	__reg_tc__("processUpdateProfile.checkText",
		"���� ��������� ����������");
	__reg_tc__("processUpdateProfile.multipleMessages",
		"���� � ������������ ��������� �������� ��������� ������� ������ �����������, ����� ������������ ��������� sms");
	__reg_tc__("processUpdateProfile.reportOptions",
		"������������� �� ��������� �������� ��������� ����������� � ��������");
	__reg_tc__("processUpdateProfile.reportOptions.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � 7-bit ���������, ��� ������� ������� �������� �������� ���� � 7-bit (��������������), ���� � UCS2 � ����������� �� ������� �������� �������");
	__reg_tc__("processUpdateProfile.codePage",
		"������������� �� ��������� �������� ������ ������ ��������� �� ������� �����");
	__reg_tc__("processUpdateProfile.codePage.dataCoding",
		"����� ��������� � ����� ���������");
	__reg_tc__("processUpdateProfile.incorrectCmdText",
		"����������� � ������������ ������ �������");
	__reg_tc__("processUpdateProfile.incorrectCmdText.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � 7-bit ���������, ��� ������� ������� �������� �������� ���� � 7-bit (��������������), ���� � UCS2 � ����������� �� ������� �������� �������");
}

public:
	SmppProfilerCheckList()
		: SmppProtocolCheckList("���������� ��������������� ������������ Profiler ����� �������� SMPP", "smpp_profiler.chk")
	{
		allProfilerTc();
	}

protected:
	SmppProfilerCheckList(const char* name, const char* fileName)
		: SmppProtocolCheckList(name, fileName)
	{
		allProfilerTc();
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_CHECK_LIST */

