#ifndef TEST_DBSME_DBSME_CHECK_LIST
#define TEST_DBSME_DBSME_CHECK_LIST

#include "test/sme/SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::test::sme::SmppProtocolCheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class DbSmeCheckList : public SmppProtocolCheckList
{

void correctJobTc()
{
	__reg_tc__("submitDbSmeCmd.correct.job",
		"�������� ������ ����������� ��������� ���� �����");
	__reg_tc__("submitDbSmeCmd.correct.job.insert",
		"���������� insert ������� � �������");
	__reg_tc__("submitDbSmeCmd.correct.job.update",
		"���������� update �������");
	__reg_tc__("submitDbSmeCmd.correct.job.delete",
		"���������� delete ������� �� �������");
	__reg_tc__("submitDbSmeCmd.correct.job.select",
		"���������� ������� ������� �� �������");
}

void correctInputTc()
{
	//input
	__reg_tc__("submitDbSmeCmd.correct.input",
		"�������� ������ � ���������� ������ ����������");
	__reg_tc__("submitDbSmeCmd.correct.input.noParams",
		"������� ��� ����������");
	__reg_tc__("submitDbSmeCmd.correct.input.int",
		"���� �� ���������� �������� ������ ���� int16 ��� int32");
	__reg_tc__("submitDbSmeCmd.correct.input.float",
		"���� �� ���������� �������� ������ ���� float ��� double");
	__reg_tc__("submitDbSmeCmd.correct.input.string",
		"���� �� ���������� �������� ������� (GSM 03.38) �� ������ ����� (��� ��������)");
	__reg_tc__("submitDbSmeCmd.correct.input.quotedString",
		"���� �� ���������� �������� ������� (GSM 03.38) � ������� �������� (������� �������)");
	__reg_tc__("submitDbSmeCmd.correct.input.date",
		"���� �� ���������� �������� ����� ��� ��������");
	//defaultInput
	__reg_tc__("submitDbSmeCmd.correct.defaultInput",
		"�������� ������ � �������������� ���������� ��� ���������� ����������� �������� �� ���������");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.int",
		"�������� �� ��������� ��� ������������� ���������� ���� int16 ��� int32");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.float",
		"�������� �� ��������� ��� ���������� ���� float ��� double");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.string",
		"�������� �� ��������� ��� ��������� (GSM 03.38) ����������");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.date",
		"�������� �� ��������� ��� ���������� ���� ���� ��� �������");
}

void incorrectInputTc()
{
	__reg_tc__("submitDbSmeCmd.incorrect.input",
		"�������� ������ � ������������� �����������");
	__reg_tc__("submitDbSmeCmd.incorrect.input.missingParams",
		"����������� ������������ ���������");
	__reg_tc__("submitDbSmeCmd.incorrect.input.extraParams",
		"������������ ������ ���������");
	__reg_tc__("submitDbSmeCmd.incorrect.input.intMismatch",
		"������������ int32, ��������� int16");
	__reg_tc__("submitDbSmeCmd.incorrect.input.floatIntMismatch",
		"������������ float ��� double, ��������� int16 ��� int32");
	__reg_tc__("submitDbSmeCmd.incorrect.input.stringIntMismatch",
		"������������ ������, ��������� �����");
	__reg_tc__("submitDbSmeCmd.incorrect.input.stringDateMismatch",
		"������������ ������, ��������� ����");
	__reg_tc__("submitDbSmeCmd.incorrect.input.dateMismatch",
		"������������ ����, ��������� ���� ����, �� � ������ �������");
	__reg_tc__("submitDbSmeCmd.incorrect.input.stringTooLong",
		"����� ������ ������ ����� ����");
	__reg_tc__("submitDbSmeCmd.incorrect.input.numTooLong",
		"����������� ����� ������ ����� ����");
}

void processDbSmeRes()
{
	__reg_tc__("processDbSmeRes", "�������� ��������� �� db sme");
	__reg_tc__("processDbSmeRes.dataCoding",
		"�������� ��������� �������� � ��������� ��������� SC");
	//processDbSmeRes.input
	__reg_tc__("processDbSmeRes.input",
		"�������� ���������� ��������� ������������ �� input");
	__reg_tc__("processDbSmeRes.input.int",
		"��������� ���� int16 ��� int32");
	__reg_tc__("processDbSmeRes.input.float",
		"��������� ���� float ��� double ��������� ������������ �� input � ������ �������");
	__reg_tc__("processDbSmeRes.input.string",
		"��������� ���������");
	__reg_tc__("processDbSmeRes.input.date",
		"���� � ����� ��������� ������������ �� input � ������ �������");
	__reg_tc__("processDbSmeRes.input.fromAddress",
		"�������� ��������� ������ ����������� from-address ����������� ���������");
	//processDbSmeRes.defaultInput
	__reg_tc__("processDbSmeRes.defaultInput",
		"��� ������������� input ���������� ������� �������� �� ���������");
	__reg_tc__("processDbSmeRes.defaultInput.int",
		"��������� �� ��������� ��� ������������� ���������� ���� int16 ��� int32");
	__reg_tc__("processDbSmeRes.defaultInput.float",
		"��������� �� ��������� ��� ���������� ���� float ��� double");
	__reg_tc__("processDbSmeRes.defaultInput.string",
		"��������� �� ��������� ��� ��������� (GSM 03.38) ����������");
	__reg_tc__("processDbSmeRes.defaultInput.now",
		"�������� �� ��������� now ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.defaultInput.today",
		"�������� �� ��������� today ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.defaultInput.yesterday",
		"�������� �� ��������� yesterday ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.defaultInput.tomorrow",
		"�������� �� ��������� tomorrow ��� ���� ����������� ���������");
	//processDbSmeRes.output
	__reg_tc__("processDbSmeRes.output",
		"����� ��������� ������������� ������� (�������� �� ��, ������������ �����, ������ ����, escape �������");
	__reg_tc__("processDbSmeRes.longOutput",
		"� ������ �������� ������� ��������� ��������� db sme ��������� ������������ ����� �� ���������� sms");
	__reg_tc__("processDbSmeRes.longOutputLimit",
		"� ������ �������� ������� ��������� ��������� db sme ��������� ������������ ���������� sms");
	__reg_tc__("processDbSmeRes.output.dateFormat",
		"���� ��������� ������������� �� ������ input ������� � ������ output ������");
	__reg_tc__("processDbSmeRes.output.fromAddress",
		"�������� ��������� ������ ����������� from-address ����������� ���������");
	__reg_tc__("processDbSmeRes.output.string",
		"�������������� ��������� ����� ����������� ���������");
	__reg_tc__("processDbSmeRes.output.string.left",
		"������������ �� ������ ����");
	__reg_tc__("processDbSmeRes.output.string.right",
		"������������ �� ������� ����");
	__reg_tc__("processDbSmeRes.output.string.center",
		"������������ �� ������");
	//processDbSmeRes.select
	__reg_tc__("processDbSmeRes.select",
		"������� �� ������� ������");
	__reg_tc__("processDbSmeRes.select.defaultOutput",
		"��� ���������� ������� ��������������� �������� ������� ��� ������� (NULL) ��������� ����� ��� output ���������� ������� �������� �� ���������");
	__reg_tc__("processDbSmeRes.select.defaultOutput.int",
		"��������� �� ��������� ��� ������������� ���������� ���� int16 ��� int32");
	__reg_tc__("processDbSmeRes.select.defaultOutput.float",
		"��������� �� ��������� ��� ���������� ���� float ��� double");
	__reg_tc__("processDbSmeRes.select.defaultOutput.string",
		"��������� �� ��������� ��� ��������� (GSM 03.38) ����������");
	__reg_tc__("processDbSmeRes.select.defaultOutput.now",
		"�������� �� ��������� now ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.select.defaultOutput.today",
		"�������� �� ��������� today ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.select.defaultOutput.yesterday",
		"�������� �� ��������� yesterday ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.select.defaultOutput.tomorrow",
		"�������� �� ��������� tomorrow ��� ���� ����������� ���������");
	__reg_tc__("processDbSmeRes.select.noRecordNoDefaults",
		"��� ���������� ������� ��������������� �������� ������� � ���������� �������� �� ���������, ������������ ��������� �� ������");
	__reg_tc__("processDbSmeRes.select.nullsNoDefaults",
		"���� � �����-���� ������ �� ���������� ������� ���� ���� �� ��������� NULL, ��� �������� ��� �������� �� ���������, ����� ��� ������ ������ ������������ ��������� �� ������");
	__reg_tc__("processDbSmeRes.select.singleRecord",
		"���� �������� ������� ������������� ������������ ������, �������� ����� � output ������� �� ����������� �������");
	__reg_tc__("processDbSmeRes.select.multipleRecords",
		"���� ��������� ������� ��������������� �������� �������, �������� ����� � output ������� ��� ������ ������ � ���������� ������ �������");
	//processDbSmeRes.insert
	__reg_tc__("processDbSmeRes.insert",
		"������� �� ���������� (insert) ������� � �������");
	__reg_tc__("processDbSmeRes.insert.ok",
		"��� �������� ���������� �������� db sme ���������� ������������� �������� output �������");
	__reg_tc__("processDbSmeRes.insert.duplicateKey",
		"��� ��������� ������������ ������ ������� db sme ���������� ��������� �� ������");
	//processDbSmeRes.update
	__reg_tc__("processDbSmeRes.update",
		"������� �� ���������� (update) ������� � �������");
	__reg_tc__("processDbSmeRes.update.ok",
		"��� �������� ���������� �������� db sme ���������� ������������� �������� output �������");
	__reg_tc__("processDbSmeRes.update.duplicateKey",
		"��� ��������� ������������ ������ ������� db sme ���������� ��������� �� ������");
	__reg_tc__("processDbSmeRes.update.recordsAffected",
		"���������� ����������� ������� �������������� ���������");
	//processDbSmeRes.delete
	__reg_tc__("processDbSmeRes.delete",
		"������� �� �������� (delete) ������� �� �������");
	__reg_tc__("processDbSmeRes.delete.ok",
		"��� �������� ���������� �������� db sme ���������� ������������� �������� output �������");
	__reg_tc__("processDbSmeRes.delete.failure",
		"��� ������������� �������� ������� db sme ���������� ��������� �� ������");
	__reg_tc__("processDbSmeRes.delete.recordsAffected",
		"���������� ��������� ������� �������������� ���������");
	//processDbSmeRes.errors
	__reg_tc__("processDbSmeRes.errors",
		"��� ������������ ������ db sme ���������� ��������������� ��������� �� �������");
	__reg_tc__("processDbSmeRes.errors.invalidJob",
		"������������ ��� ������");
	__reg_tc__("processDbSmeRes.errors.invalidInput",
		"����������� �������� ���������");
	__reg_tc__("processDbSmeRes.errors.invalidConfig",
		"������������ ������������ ������ (sql, input, output)");
}

public:
	DbSmeCheckList()
		: SmppProtocolCheckList("���������� ��������������� ������������ db sme", "dbsme.chk")
	{
		//submitDbSmeCmd.correct
		__reg_tc__("submitDbSmeCmd",
			"�������� sms �� db sme");
		__reg_tc__("submitDbSmeCmd.cmdTextDefault",
			"����� ������� � ��������� ��������� SC");
		__reg_tc__("submitDbSmeCmd.cmdTextUcs2",
			"����� ������� � ��������� UCS2");
		__reg_tc__("submitDbSmeCmd.correct", "�������� ���������� ������� db sme");
		correctJobTc();
		correctInputTc();
		//submitDbSmeCmd.incorrect
		__reg_tc__("submitDbSmeCmd.incorrect", "�������� ������������ ������� db sme");
		incorrectInputTc();
		//result
		processDbSmeRes();
	}
};

}
}
}

#endif /* TEST_DBSME_DBSME_CHECK_LIST */

