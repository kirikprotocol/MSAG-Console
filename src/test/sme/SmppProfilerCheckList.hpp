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
public:
	SmppProfilerCheckList()
		: SmppProtocolCheckList("���������� ��������������� ������������ Profiler ����� �������� SMPP", "smpp_profiler.chk")
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
		__reg_tc__("processUpdateProfile.reportOptions",
			"������������� �� ��������� �������� ��������� ����������� � ��������");
		__reg_tc__("processUpdateProfile.codePage",
			"������������� �� ��������� �������� ������ ������ ��������� �� ������� �����");
		__reg_tc__("processUpdateProfile.incorrectCmdText",
			"����������� � ������������ ������ �������");
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_CHECK_LIST */

