#ifndef TEST_SME_SYSTEM_SME_CHECK_LIST
#define TEST_SME_SYSTEM_SME_CHECK_LIST

#include "SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SystemSmeCheckList : public SmppProtocolCheckList
{
void profilerTc()
{
	//updateProfile
	__reg_tc__("updateProfile",
		"�������� smpp ��������� ��������� ��������");
	__reg_tc__("updateProfile.submitSm",
		"�������� submit_sm pdu");
	__reg_tc__("updateProfile.dataSm",
		"�������� data_sm pdu");
	__reg_tc__("updateProfile.cmdTextDefault",
		"����� ������� � ��������� ��������� SC");
	__reg_tc__("updateProfile.cmdText7bit",
		"����� ������� � 7bit ��������� SC");
	__reg_tc__("updateProfile.cmdTextUcs2",
		"����� ������� � ��������� UCS2");
	__reg_tc__("updateProfile.cmdTextMixedCase",
		"����� ������� � ��������� ��������");
	__reg_tc__("updateProfile.cmdTextExtraWhiteSpaces",
		"����� ������� � ������� ��������� (� ������, ��������, �����)");
	__reg_tc__("updateProfile.reportOptions",
		"��������� ����� ����������� � ��������");
	__reg_tc__("updateProfile.reportOptions.reportNone",
		"���������� �� ����������� � �������� (report none)");
	__reg_tc__("updateProfile.reportOptions.reportFull",
		"���������� ����� ����������� � �������� (report full)");
	__reg_tc__("updateProfile.reportOptions.reportFinal",
		"���������� ����� ����������� � ���������� �������� (report final)");
	__reg_tc__("updateProfile.reportOptions.reportNoneUssd",
		"���������� �� ����������� � �������� �������� ussd (*50*0#)");
	__reg_tc__("updateProfile.reportOptions.reportFullUssd",
		"���������� ����� ����������� � �������� �������� ussd (*50*2#)");
	__reg_tc__("updateProfile.reportOptions.reportFinalUssd",
		"���������� ����� ����������� � ���������� �������� �������� ussd (*50*1#)");
	__reg_tc__("updateProfile.dataCoding",
		"��������� ������ ������ ���������");
	__reg_tc__("updateProfile.dataCoding.ucs2",
		"���������� ����� ������ ��������� �� ������� ����� (ucs2)");
	__reg_tc__("updateProfile.dataCoding.default",
		"���������� �� ������ ������ ��������� �� ������� ����� (default)");
	__reg_tc__("updateProfile.dataCoding.ucs2Ussd",
		"���������� ����� ������ ��������� �� ������� ����� �������� ussd (*50*4#)");
	__reg_tc__("updateProfile.dataCoding.defaultUssd",
		"���������� �� ������ ������ ��������� �� ������� ����� �������� ussd (*50*3#)");
	__reg_tc__("updateProfile.locale",
		"��������� �������� ������");
	__reg_tc__("updateProfile.locale.existentLocale",
		"��������� �������� ������ (locale en_us, locale ru_ru)");
	__reg_tc__("updateProfile.locale.nonExistentLocale",
		"�������� ������ �� ��������� � ������ ������� �������������� SC");
	__reg_tc__("updateProfile.locale.enLocaleUssd",
		"���������� ������ en_us �������� ussd (*50*21#)");
	__reg_tc__("updateProfile.locale.ruLocaleUssd",
		"���������� ������ ru_ru �������� ussd (*50*20#)");
	__reg_tc__("updateProfile.locale.nonExistentLocaleUssd",
		"�������� ������ �� ��������� � ������ ussd ��������");
	__reg_tc__("updateProfile.hide",
		"��������� hide �����");
	__reg_tc__("updateProfile.hide.hide",
		"���������� ����� hide (������� hide)");
	__reg_tc__("updateProfile.hide.unhide",
		"���������� ����� unhide (������� unhide)");
	__reg_tc__("updateProfile.hide.hideUssd",
		"���������� ����� hide �������� ussd (*50*5#)");
	__reg_tc__("updateProfile.hide.unhideUssd",
		"���������� ����� unhide �������� ussd (*50*6#)");
	__reg_tc__("updateProfile.hide.hideDenied",
		"������� �������� ����� hide, ���� ��� ������� ��������� ��������� hide �����");
	__reg_tc__("updateProfile.incorrectCmdText",
		"������������ ����� �������");
	__reg_tc__("updateProfile.incorrectUssdCmd",
		"������������ ussd ������");
	__reg_tc__("updateProfile.incorrectUssdServiceOp",
		"������������ �������� ���� ussd_service_op (profiler ������������� ����� ussd ������)");
	//updateProfile.ack
	__reg_tc__("updateProfile.ack",
		"�������� ��������� �� ��������� ��������");
	__reg_tc__("updateProfile.ack.checkFields",
		"���� pdu ������������� ���������� ��������� ��������");
	__reg_tc__("updateProfile.ack.checkText",
		"���� ��������� ����������");
	__reg_tc__("updateProfile.ack.reportOptions",
		"������������� �� ��������� �������� ��������� ����������� � ��������");
	__reg_tc__("updateProfile.ack.reportOptions.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � default ���������, ��� ������� ������� �������� �������� ���� � default (��������������), ���� � ucs2 � ����������� �� ������� �������� �������");
	__reg_tc__("updateProfile.ack.codePage",
		"������������� �� ��������� �������� ������ ������ ��������� �� ������� �����");
	__reg_tc__("updateProfile.ack.codePage.dataCoding",
		"����� ��������� � ����� ��������� (ucs2 ��� default)");
	__reg_tc__("updateProfile.ack.locale",
		"������������� �� ��������� �������� ������");
	__reg_tc__("updateProfile.ack.locale.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � default ���������, ��� ������� ������� �������� �������� ���� � default (��������������), ���� � ucs2 � ����������� �� ������� �������� �������");
	__reg_tc__("updateProfile.ack.hide",
		"������������� �� ��������� hide �����");
	__reg_tc__("updateProfile.ack.hide.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � default ���������, ��� ������� ������� �������� �������� ���� � default (��������������), ���� � ucs2 � ����������� �� ������� �������� �������");
	__reg_tc__("updateProfile.ack.hideDenied",
		"��������� �� ������ ��� ������� ��������� hide �����, ���� ��� ������� ��������� ��������� hide");
	__reg_tc__("updateProfile.ack.hideDenied.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � default ���������, ��� ������� ������� �������� �������� ���� � default (��������������), ���� � ucs2 � ����������� �� ������� �������� �������");
	__reg_tc__("updateProfile.ack.incorrectCmdText",
		"����������� � ������������ ������ �������");
	__reg_tc__("updateProfile.ack.incorrectCmdText.dataCoding",
		"��� ���������� � ������ ��������� ������� �������� �������� � 7-bit ���������, ��� ������� ������� �������� �������� ���� � 7-bit (��������������), ���� � UCS2 � ����������� �� ������� �������� �������");
}

void abonentInfoTc()
{
	//queryAbonentInfo
	__reg_tc__("queryAbonentInfo",
		"�������� smpp ��������� abonent info sme");
	__reg_tc__("queryAbonentInfo.submitSm",
		"�������� submit_sm pdu");
	__reg_tc__("queryAbonentInfo.dataSm",
		"�������� data_sm pdu");
	__reg_tc__("queryAbonentInfo.cmdTextDefault",
		"����� ������� � ��������� ��������� SC");
	__reg_tc__("queryAbonentInfo.cmdText7bit",
		"����� ������� � 7bit ��������� SC");
	__reg_tc__("queryAbonentInfo.cmdTextUcs2",
		"����� ������� � ��������� UCS2");
	__reg_tc__("queryAbonentInfo.smppAddr",
		"�������� ������� �� smpp ����� abonent info sme");
	__reg_tc__("queryAbonentInfo.mapAddr",
		"�������� ������� �� map (mobile) ����� abonent info sme");
	//queryAbonentInfo.correct
	__reg_tc__("queryAbonentInfo.correct",
		"�������� ���������� �������� �� abonent info sme");
	__reg_tc__("queryAbonentInfo.correct.existentAddr",
		"������ ������� ������������� ��������");
	__reg_tc__("queryAbonentInfo.correct.nonExistentAddr",
		"������ ������� ��������������� ��������");
	__reg_tc__("queryAbonentInfo.correct.unifiedAddrFormat",
		"������� � ������� � ��������������� (.ton.npi.addr) �������");
	__reg_tc__("queryAbonentInfo.correct.humanFormat",
		"������� � ������� � ����������� (+7902... ��� 83832...) ������");
	//queryAbonentInfo.incorrect
	__reg_tc__("queryAbonentInfo.incorrect",
		"�������� ������������ �������� �� abonent info sme");
	__reg_tc__("queryAbonentInfo.incorrect.addrFormat",
		"������������ ������ ������");
	__reg_tc__("queryAbonentInfo.incorrect.addrLength",
		"����� ������ ������ ������������ (20 ��������)");
	__reg_tc__("queryAbonentInfo.incorrect.invalidSymbols",
		"������������ ������� � ������ ������");
	__reg_tc__("queryAbonentInfo.incorrect.extraWords",
		"� ������ ������� ������ ����� (������ ���� ������ �����)");
	__reg_tc__("queryAbonentInfo.incorrect.tonNpi",
		"������������ ton �/��� npi � ������");
	//queryAbonentInfo.ack
	__reg_tc__("queryAbonentInfo.ack",
		"�������� ��������� �� abonent info sme");
	__reg_tc__("queryAbonentInfo.ack.checkFields",
		"���� pdu ������������� ���������� abonent info sme");
	__reg_tc__("queryAbonentInfo.ack.checkText",
		"���� ��������� ����������");
}

void smscSmeTc()
{
	//updateProfile
	__reg_tc__("submitPduToSmscSme",
		"�������� submit_sm �� smsc sme");
	__reg_tc__("dataPduToSmscSme",
		"�������� data_sm �� smsc sme");
}

public:
	SystemSmeCheckList()
	: SmppProtocolCheckList("���������� ��������������� ������������ Profiler, AbonentInfo � SmscSme ����� �������� SMPP", "system_sme.chk")
	{
		profilerTc();
		abonentInfoTc();
		smscSmeTc();
	}

protected:
	SystemSmeCheckList(const char* name, const char* fileName)
	: SmppProtocolCheckList(name, fileName)
	{
		profilerTc();
		abonentInfoTc();
		smscSmeTc();
	}
};

}
}
}

#endif /* TEST_SME_SYSTEM_SME_CHECK_LIST */

