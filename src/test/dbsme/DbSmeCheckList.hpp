#ifndef TEST_DBSME_DBSME_CHECK_LIST
#define TEST_DBSME_DBSME_CHECK_LIST

#include "test/sme/SystemSmeCheckList.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::test::sme::SystemSmeCheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
#define __hide_tc__(id) \
	hideTc(id)

class DbSmeCheckList : public SystemSmeCheckList
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
		"���������� select ������� �� �������");
	__reg_tc__("submitDbSmeCmd.correct.job.procedure",
		"����� �������� ��������� �� PL/SQL");
	__reg_tc__("submitDbSmeCmd.correct.job.function",
		"����� ������� �� PL/SQL");
}

void correctInputTc()
{
	//input
	__reg_tc__("submitDbSmeCmd.correct.input",
		"�������� ������ � ���������� ������ ����������");
	__reg_tc__("submitDbSmeCmd.correct.input.noParams",
		"������� ��� ����������");
	__reg_tc__("submitDbSmeCmd.correct.input.int",
		"���� �� ���������� �������� ������ ���� int8, int16, int32 ��� int64");
	__reg_tc__("submitDbSmeCmd.correct.input.uint",
		"���� �� ���������� �������� ������ ���� uint8, uint16, uint32 ��� uint64");
	__reg_tc__("submitDbSmeCmd.correct.input.float",
		"���� �� ���������� �������� ������ ���� float, double ��� long double");
	__reg_tc__("submitDbSmeCmd.correct.input.string",
		"���� �� ���������� �������� ������� (GSM 03.38) �� ������ ����� (��� ��������)");
	__reg_tc__("submitDbSmeCmd.correct.input.quotedString",
		"���� �� ���������� �������� ������� (GSM 03.38) � ������� �������� (������� �������)");
	__reg_tc__("submitDbSmeCmd.correct.input.date",
		"���� �� ���������� �������� ����� ��� ��������");
	__reg_tc__("submitDbSmeCmd.correct.input.jobName",
		"���� �� ���������� �������� ������ ������ job-name");
	__reg_tc__("submitDbSmeCmd.correct.input.toAddress",
		"���� �� ���������� �������� ������� ���������� to-address");
	__reg_tc__("submitDbSmeCmd.correct.input.fromAddress",
		"���� �� ���������� �������� ������� ����������� from-address");
	//defaultInput
	__reg_tc__("submitDbSmeCmd.correct.defaultInput",
		"�������� ������ � �������������� ���������� ��� ���������� ����������� �������� �� ���������");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.int",
		"�������� �� ��������� ��� ������������� ���������� ���� int8, int16, int32 ��� int64");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.uint",
		"�������� �� ��������� ��� ������������� ���������� ���� uint8, uint16, uint32 ��� uint64");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.float",
		"�������� �� ��������� ��� ���������� ���� float, double ��� long double");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.string",
		"�������� �� ��������� ��� ��������� (GSM 03.38) ����������");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.date",
		"�������� �� ��������� ��� ���������� ���� ���� ��� �������");
}

void incorrectInputTc()
{
	__reg_tc__("submitDbSmeCmd.incorrect", "�������� ������������ ������� db sme");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidJob",
		"������������ ��� ������");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidAddr",
		"������������ ����� (�� ��������������� data provider)");
	__reg_tc__("submitDbSmeCmd.incorrect.missingParams",
		"����������� ������������ ���������");
	__reg_tc__("submitDbSmeCmd.incorrect.extraParams",
		"������������ ������ ���������");
	//dateFormat
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat",
		"������ ���� �� ������������� ����������");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidDay",
		"������������ ������ ����� (����� 0, ������ 31 � �.�.)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth",
		"����������� ����� ����� (����� 0, ������ 13, ��������� Jan, �������� 01 � �.�.)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidYear",
		"����������� ����� ��� (�� ������� ��������� 2002, �������� 02 � ��������)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour",
		"����������� ����� ��� (0 am, 0 pm, 13am, 13 pm, 24 � ������)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMinute",
		"����������� ������ ������ (60 � ������)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidSecond",
		"����������� ������ ������� (60 � ������)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidIndicator",
		"����������� ����� ��������� am/pm");
	//invalidNumber
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber",
		"������������ �������� �������� ����������");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber.int",
		"������������ �������� ���������� int8, int16, int32, int64 (������ ����������� �����������, ������ ���������� �����������, ���������������, ������ � �.�.)");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber.uint",
		"������������ �������� ���������� uint8, uint16, uint32, uint64 (������ ����������� �����������, ������ ���������� �����������, ���������������, ������ � �.�.)");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber.float",
		"������������ �������� ���������� float, double, long-double (������ ����������� �����������, ������ � �.�.)");
	//
	__reg_tc__("submitDbSmeCmd.incorrect.input.stringTooLong",
		"����� ������ ������ ����� ����");
}

void processDbSmeRes()
{
	__reg_tc__("processDbSmeRes", "�������� ��������� �� db sme");
	__reg_tc__("processDbSmeRes.checkFields",
		"�������� ����� pdu source_addr, data_coding, service_type, protocol_id ������������� ���������� db sme");
	__reg_tc__("processDbSmeRes.longOutput",
		"� ������ �������� ������� ��������� ��������� db sme ��������� ������������ ����� �� ���������� sms");
	__reg_tc__("processDbSmeRes.longOutputLimit",
		"� ������ �������� ������� ��������� ��������� db sme ��������� ������������ ���������� sms");
	//processDbSmeRes.input
	__reg_tc__("processDbSmeRes.input",
		"�������� ���������� ��������� ������������ �� input");
	__reg_tc__("processDbSmeRes.input.int",
		"��������� ���� int8, int16, int32 � int64");
	__reg_tc__("processDbSmeRes.input.uint",
		"��������� ���� uint8, uint16, uint32 � uint64");
	__reg_tc__("processDbSmeRes.input.float",
		"��������� ���� float, double � long double � ������ �������");
	__reg_tc__("processDbSmeRes.input.string",
		"��������� ���������");
	__reg_tc__("processDbSmeRes.input.date",
		"���� � ����� � ������ �������");
	__reg_tc__("processDbSmeRes.input.jobName",
		"�������� ��������� ����� ������ job-name ����������� ���������");
	__reg_tc__("processDbSmeRes.input.toAddress",
		"�������� ��������� ������ ���������� to-address ����������� ���������");
	__reg_tc__("processDbSmeRes.input.fromAddress",
		"�������� ��������� ������ ����������� from-address ����������� ���������");
	//processDbSmeRes.defaultInput
	__reg_tc__("processDbSmeRes.defaultInput",
		"��� ������������� input ���������� ������� �������� �� ���������");
	__reg_tc__("processDbSmeRes.defaultInput.int",
		"��������� �� ��������� ��� ������������� ���������� ���� int8, int16, int32 � int64");
	__reg_tc__("processDbSmeRes.defaultInput.uint",
		"��������� �� ��������� ��� ������������� ���������� ���� uint8, uint16, uint32 � uint64");
	__reg_tc__("processDbSmeRes.defaultInput.float",
		"��������� �� ��������� ��� ���������� ���� float, double � long double");
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
		"����� ��������� ������������� ������� (�������� �� ��, ������������ �����, ������ ����, escape �������)");
	__reg_tc__("processDbSmeRes.output.dateFormat",
		"���� ��������� � ������������ � output ��������");
	__reg_tc__("processDbSmeRes.output.jobName",
		"�������� ��������� ����� ������ job-name ����������� ���������");
	__reg_tc__("processDbSmeRes.output.toAddress",
		"�������� ��������� ������ ���������� to-address ����������� ���������");
	__reg_tc__("processDbSmeRes.output.toAddressUnified",
		"�������� ��������� ������ ���������� to-address-unified ����������� ���������");
	__reg_tc__("processDbSmeRes.output.fromAddress",
		"�������� ��������� ������ ����������� from-address ����������� ���������");
	__reg_tc__("processDbSmeRes.output.fromAddressUnified",
		"�������� ��������� ������ ����������� from-address-unified ����������� ���������");
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
		"��� ���������� ������� ��������������� �������� ������� ��� ������� (NULL) ��������� ����� ������� output �������� �� ���������");
	__reg_tc__("processDbSmeRes.select.defaultOutput.int",
		"��������� �� ��������� ��� ������������� ���������� ���� int8, int16, int32, int64, uint8, uint16, uint32 � uint64");
	__reg_tc__("processDbSmeRes.select.defaultOutput.float",
		"��������� �� ��������� ��� ���������� ���� float, double � long double");
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
	__reg_tc__("processDbSmeRes.select.nullsNoDefaults",
		"���� � �����-���� ������ �� ���������� ������� ���� ���� �� ��������� NULL, ��� �������� ��� �������� �� ���������, ����� ��� ������ ������ ������������ ��������� �� ������");
	__reg_tc__("processDbSmeRes.select.noRecords",
		"��� ���������� ������� ��������������� �������� ������� ������������ ��������� �� ������");
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
	__reg_tc__("processDbSmeRes.procedure",
		"����� �������� PL/SQL �������� � in/out �����������");
	__reg_tc__("processDbSmeRes.function",
		"����� PL/SQL ������� � in/out �����������");
	__reg_tc__("processDbSmeRes.exception",
		"��������� PL/SQL �������� � ������� ������������ exception ����������� ���������");
	//processDbSmeRes.errors
	__reg_tc__("processDbSmeRes.errors",
		"��� ������������ ������ db sme ���������� ��������������� ��������� �� �������");
	__reg_tc__("processDbSmeRes.errors.invalidConfig",
		"������������ ������������ ������ (sql, input, output)");
}

public:
	DbSmeCheckList()
		: SystemSmeCheckList("���������� ��������������� ������������ db sme", "dbsme.chk")
	{
		__hide_tc__("bindIncorrectSme");
		__hide_tc__("submitSm.correct");
		__hide_tc__("submitSm.incorrect");
		__hide_tc__("submitSm.assert");
		__hide_tc__("replaceSm");
		__hide_tc__("sendDeliverySmResp.sendRetry");
		__hide_tc__("sendDeliverySmResp.sendError");
		__hide_tc__("processReplaceSmResp");
		__hide_tc__("processDeliverySm.normalSms");
		//__hide_tc__("processDeliverySm.deliveryReceipt");
		__hide_tc__("processDeliverySm.intermediateNotification");
		__hide_tc__("notImplemented");
		__hide_tc__("updateProfile");
		__hide_tc__("processUpdateProfile");

		//submitDbSmeCmd.correct
		__reg_tc__("submitDbSmeCmd", "�������� sms �� db sme");
		__reg_tc__("submitDbSmeCmd.cmdTextDefault",
			"����� ������� � ��������� ��������� SC");
		__reg_tc__("submitDbSmeCmd.cmdTextUcs2",
			"����� ������� � ��������� UCS2");
		__reg_tc__("submitDbSmeCmd.jobNameMixedCase",
			"��� ������ � ��������� �������� (��������� � ��������� �����)");
		__reg_tc__("submitDbSmeCmd.dateMixedCase",
			"���� � ��������� �������� (��������� � ��������� ����� � ����� ������ ��� AM/PM ����������)");
		__reg_tc__("submitDbSmeCmd.correct", "�������� ���������� ������� db sme");
		correctJobTc();
		correctInputTc();
		incorrectInputTc();
		//result
		processDbSmeRes();
	}
};

}
}
}

#endif /* TEST_DBSME_DBSME_CHECK_LIST */

