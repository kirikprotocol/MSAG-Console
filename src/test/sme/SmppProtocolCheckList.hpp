#ifndef TEST_SME_SMPP_PROTOCOL_CHECK_LIST
#define TEST_SME_SMPP_PROTOCOL_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmppProtocolCheckList : public CheckList
{

void bindTc()
{
	__reg_tc__("bind",
		"������������ IP ���������� � SC � �����������");
	//bind.correct
	__reg_tc__("bind.correct",
		"������������ IP ���������� � SC � ����������� � ����������� �����������");
	__reg_tc__("bind.correct.receiver",
		"����������� �������� � ������� bind_receiver pdu");
	__reg_tc__("bind.correct.transmitter",
		"����������� ������������ � ������� bind_transmitter pdu");
	__reg_tc__("bind.correct.transceiver",
		"����������� ���������� � ������� bind_transceiver pdu");
	//bind.incorrect
	__reg_tc__("bind.incorrect",
		"������������ IP ���������� � SC � ����������� � ������������� �����������");
	__reg_tc__("bind.incorrect.systemIdNotRegistered",
		"�������� system_id �� ��������� � ������������ SC");
	__reg_tc__("bind.incorrect.invalidPassword",
		"������������ ������");
	__reg_tc__("bind.incorrect.unknownHost",
		"��������� ���������� � ����������� SC (����������� ����) ����������� ���������");
	__reg_tc__("bind.incorrect.invalidPort",
		"��������� ���������� � ����������� SC (������������ ����) ����������� ���������");
	__reg_tc__("bind.incorrect.invalidSystemIdLength",
		"����� ���� system_id � bind pdu ������ ����������� ����������");
	__reg_tc__("bind.incorrect.invalidPasswordLength",
		"����� ���� password � bind pdu ������ ����������� ����������");
	__reg_tc__("bind.incorrect.invalidSystemTypeLength",
		"����� ���� system_type � bind pdu ������ ����������� ����������");
	__reg_tc__("bind.incorrect.invalidAddressRangeLength",
		"����� ���� address_range � bind pdu ������ ����������� ����������");
	//bind.resp
	__reg_tc__("bind.resp.checkCommandStatus",
		"� bind �������� �������� ���� command_status ����� ESME_ROK");
	__reg_tc__("bind.resp.checkInterfaceVersion",
		"� bind �������� �������� ���� sc_interface_version ����� 0x34");
}

void unbindTc()
{
	__reg_tc__("unbind", "���������� ���������� � SC");
	__reg_tc__("unbind.resp.checkCommandStatus",
		"� unbind �������� �������� ���� command_status ����� ESME_ROK");
	__reg_tc__("unbind.resp.checkSocketClose",
		"����� �������� unbind �������� SC ��������� ����������");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "���� ����� ��� submit_sm");
	__reg_tc__("submitSm.sync", "�������� ����������� submit_sm pdu");
	__reg_tc__("submitSm.async", "�������� ������������ submit_sm pdu");
	//submitSm.correct
	__reg_tc__("submitSm.correct",
		"�������� submit_sm pdu � ����������� ���������� �����");
	__reg_tc__("submitSm.correct.serviceTypeMarginal",
		"���������� �������� ����� ���� service_type");
	__reg_tc__("submitSm.correct.waitTimePast",
		"����� schedule_delivery_time � ������� (�������� ��� ������ ���� ��������)");
	__reg_tc__("submitSm.correct.validTimeExceeded",
		"����� validity_period ������ ����������� ����� ���������� �������������� ��� SC");
	__reg_tc__("submitSm.correct.waitTimeImmediate",
		"���������� ����������� �������� ��������� (schedule_delivery_time = NULL)");
	__reg_tc__("submitSm.correct.validTimeDefault",
		"���������� ���� ���������� �� ��������� (validity_period = NULL)");
	__reg_tc__("submitSm.correct.smLengthMarginal",
		"���������� �������� ����� ���� ���������");
	__reg_tc__("submitSm.correct.checkMap",
		"������������� ��������� �������� MAP (��������� source_addr, dest_addr � user_message_reference), �� �������� SMPP ������������ ��������� ����������� � ������� �������� �������� ���� �� �����");
	__reg_tc__("submitSm.correct.notReplace",
		"������������� ��������� (��������� source_addr, dest_addr � service_type) ��� ��������� �������������, ������������ � ������� ��������");
	__reg_tc__("submitSm.correct.serviceTypeNotMatch",
		"���� ��������� source_addr � dest_addr, �� ���������� service_type ��� ��������� ������������ � ������� ��������, �� ��������� ����� ���������");
	__reg_tc__("submitSm.correct.sourceAddrNotMatch",
		"���� ��������� dest_addr � service_type, �� ���������� source_addr ��� ��������� ������������ � ������� ��������, �� ��������� ����� ���������");
	__reg_tc__("submitSm.correct.destAddrNotMatch",
		"���� ��������� source_addr � service_type, �� ���������� dest_addr ��� ��������� ������������ � ������� ��������, �� ��������� ����� ���������");
	__reg_tc__("submitSm.correct.replaceEnrote",
		"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� �������������, ������������ � ������� ��������");
	__reg_tc__("submitSm.correct.replaceReplacedEnrote",
		"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� ��� ����� ����������� � ������������ � ������� ��������");
	__reg_tc__("submitSm.correct.replaceFinal",
		"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� ��� �������������");
	__reg_tc__("submitSm.correct.replaceRepeatedDeliveryEnrote",
		"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� �������������, ������������ � ������� ��������� ��������");
	//submitSm.incorrect
	__reg_tc__("submitSm.incorrect",
		"�������� submit_sm pdu � ������������� ���������� �����");
	__reg_tc__("submitSm.incorrect.sourceAddr",
		"������������ ����� �����������");
	__reg_tc__("submitSm.incorrect.destAddr",
		"������������ ����� ����������");
	__reg_tc__("submitSm.incorrect.validTimeFormat",
		"������������ ������ validity_period");
	__reg_tc__("submitSm.incorrect.waitTimeFormat",
		"������������ ������ schedule_delivery_time");
	__reg_tc__("submitSm.incorrect.validTimePast",
		"����� validity_period � ������� (��������� ������� �� ����� ����������)");
	__reg_tc__("submitSm.incorrect.waitTimeInvalid1",
		"�������� schedule_delivery_time ������ validity_period");
	__reg_tc__("submitSm.incorrect.waitTimeInvalid2",
		"�������� schedule_delivery_time ������ ������������� validity_period ��� SC, ���� ���������� �� ��������� (validity_period = NULL)");
	__reg_tc__("submitSm.incorrect.waitTimeInvalid3",
		"�������� schedule_delivery_time ������ validity_period, �� ��� ������ ������������� validity_period ��� SC");
	__reg_tc__("submitSm.incorrect.dataCoding",
		"������������ �������� dataCoding");
	__reg_tc__("submitSm.incorrect.transactionRollback",
		"����������� sms ����������� transaction rollback �� ������� ������� ��");
	//submitSm.assert
	__reg_tc__("submitSm.assert",
		"���������� � �������� submit_sm pdu � ������������� ���������� �����");
	__reg_tc__("submitSm.assert.serviceTypeInvalid",
		"������������ �������� ����� ���� service_type");
	__reg_tc__("submitSm.assert.destAddrLengthInvalid",
		"������������ �������� ����� ���� ������ ����������");
	__reg_tc__("submitSm.assert.msgLengthInvalid",
		"������������ �������� ����� ������ ��������� short_message");
	//submitSm.resp
	__reg_tc__("submitSm.resp",
		"��������� submit_sm_resp pdu");
	__reg_tc__("submitSm.resp.sync",
		"��������� submit_sm_resp pdu ��� ���������� submit_sm ��������");
	__reg_tc__("submitSm.resp.async",
		"��������� submit_sm_resp pdu ��� ����������� submit_sm ��������");
	__reg_tc__("submitSm.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("submitSm.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("submitSm.resp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("submitSm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status, ����������� ��� ������� ��� ���������� �������� ��������� (���� �������� ������ ���������, ���������� ������� � �.�.)");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidDestAddr",
		"���� ��� ������ ESME_RINVDSTADR � ���� command_status, �� �������� ������������� �� ����������");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidWaitTime",
		"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� ����� schedule_delivery_time ������������� ������ �����������");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidValidTime",
		"���� ��� ������ ESME_RINVEXPIRY � ���� command_status, �� ����� validity_period ������������� ������ �����������");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidDataCoding",
		"���� ��� ������ ESME_RINVDCS � ���� command_status, �� ��������� ��������� ������������� ������ �����������");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidServiceType",
		"���� ��� ������ ESME_RINVSERTYP � ���� command_status, �� ����� ���� service_type ������������� ������ �����������");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� ����� ����������� ������������� �� ������������� address range ��� ������ sme � ������������ sme.xml SC");
	__reg_tc__("submitSm.resp.checkCmdStatusSystemError",
		"���� ��� ������ ESME_RSYSERR � ���� command_status, �� �� ������� SC ������������� �������� ������������ ������ (transaction rollback ��� ���������� ���������)");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("submitSm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void replaceSmTc()
{
	__reg_tc__("replaceSm", "���� ����� ��� replace_sm");
	__reg_tc__("replaceSm.sync",
		"�������� ����������� replace_sm pdu");
	__reg_tc__("replaceSm.async",
		"�������� ������������ replace_sm pdu");
	//replaceSm.correct
	__reg_tc__("replaceSm.correct",
		"�������� replace_sm pdu � ����������� ���������� �����");
	__reg_tc__("replaceSm.correct.waitTimePast",
		"����� schedule_delivery_time � ������� (�������� ��� ������ ���� ��������)");
	__reg_tc__("replaceSm.correct.validTimeExceeded",
		"����� validity_period ������ ����������� ����� ���������� �������������� ��� SC");
	__reg_tc__("replaceSm.correct.waitTimeNull",
		"����� ������ �������� ����� NULL (�������� ��� ���������)");
	__reg_tc__("replaceSm.correct.validTimeNull",
		"����� validity_period ����� NULL (����������� validity_period �� ���������� sms)");
	__reg_tc__("replaceSm.correct.smLengthMarginal",
		"���������� �������� ����� ���� ��������� short_message");
	__reg_tc__("replaceSm.correct.replaceReplacedEnrote",
		"��������� ��� ����� ����������� ��������� � ������������ � ������� ��������");
	__reg_tc__("replaceSm.correct.replaceRepeatedDeliveryEnrote",
	   "��������� ��������� ������������ � ������� ��������� ��������");
	//replaceSm.incorrect
	__reg_tc__("replaceSm.incorrect",
		"�������� replace_sm pdu � ������������� ���������� �����");
	__reg_tc__("replaceSm.incorrect.messageId",
		"���� message_id ������ ����������� (�������������� sms)");
	__reg_tc__("replaceSm.incorrect.sourceAddr",
		"���� message_id ����� ���������, �� ���������� source_addr, �� ������������ sms �� ����������");
	__reg_tc__("replaceSm.incorrect.validTimeFormat",
		"������������ ������ validity_period");
	__reg_tc__("replaceSm.incorrect.waitTimeFormat",
		"������������ ������ schedule_delivery_time");
	__reg_tc__("replaceSm.incorrect.validTimePast",
		"����� validity_period � ������� (��������� ������� �� ����� ����������)");
	__reg_tc__("replaceSm.incorrect.waitTimeInvalid1",
		"�������� schedule_delivery_time ������ validity_period");
	__reg_tc__("replaceSm.incorrect.waitTimeInvalid2",
		"�������� validity_period = NULL (��������� ������������ ��������), � schedule_delivery_time ������ validity_period ����������� ���������");
	__reg_tc__("replaceSm.incorrect.waitTimeInvalid3",
		"�������� schedule_delivery_time = NULL (��������� ������������ ��������), � validity_period ������ schedule_delivery_time ����������� ���������");
	__reg_tc__("replaceSm.incorrect.replaceFinal",
		"��������� ��������� ������������ � ��������� ��������� (��� ������ ESME_RREPLACEFAIL)");
	__reg_tc__("replaceSm.incorrect.transactionRollback",
		"����������� sms ����������� transaction rollback �� ������� ������� ��");
	//replaceSm.resp
	__reg_tc__("replaceSm.resp",
		"��������� replace_sm_resp pdu");
	__reg_tc__("replaceSm.resp.sync",
		"��������� replace_sm_resp pdu ��� ���������� replace_sm ��������");
	__reg_tc__("replaceSm.resp.async",
		"��������� replace_sm_resp pdu ��� ����������� replace_sm ��������");
	__reg_tc__("replaceSm.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("replaceSm.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("replaceSm.resp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("replaceSm.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("replaceSm.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("replaceSm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status, ����������� ��� ������� ��� ���������� �������� ��������� (���� �������� ������ ���������, ���������� ������� � �.�.)");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidWaitTime",
		"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� ����� schedule_delivery_time ������������� ������ �����������");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidValidTime",
		"���� ��� ������ ESME_RINVEXPIRY � ���� command_status, �� ����� validity_period ������������� ������ �����������");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� ����� ����������� ������������� �� ������������� address range ��� ������ sme � ������������ sme.xml SC");
	__reg_tc__("replaceSm.resp.checkCmdStatusSystemError",
		"���� ��� ������ ESME_RSYSERR � ���� command_status, �� �� ������� SC ������������� �������� ������������ ������ (transaction rollback ��� ���������� ���������)");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidMsgId",
		"���� ��� ������ ESME_RINVMSGID � ���� command_status, �� ������������� message_id ����� �����������");
	__reg_tc__("replaceSm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void deliverySmTc()
{
	__reg_tc__("deliverySm", "���� ����� ��� deliver_sm");
	__reg_tc__("deliverySm.checkFields",
		"����� �������� ����� ������ � ��������� ������ �����");
	//deliverySm.normalSms
	__reg_tc__("deliverySm.normalSms",
		"������� sms �� ���������� ������������");
	__reg_tc__("deliverySm.normalSms.checkAllowed",
		"�������� ������������� ��������� sms");
	__reg_tc__("deliverySm.normalSms.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ����������)");
	__reg_tc__("deliverySm.normalSms.checkMandatoryFields",
		"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm) pdu");
	__reg_tc__("deliverySm.normalSms.checkDataCoding",
		"��������� ������ sms ������������� ���������� ������� ����������");
	__reg_tc__("deliverySm.normalSms.checkTextEqualDataCoding",
		"��� ���������� ��������� � �������� ����������� � ���������� ����� sms ���������");
	__reg_tc__("deliverySm.normalSms.checkTextDiffDataCoding",
		"��� ��������� ��������� � �������� ����������� � ���������� ����� sms ��������� ������������� �� ����� ��������� � ������");
	__reg_tc__("deliverySm.normalSms.checkOptionalFields",
		"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm) pdu");
	__reg_tc__("deliverySm.normalSms.scheduleChecks",
		"���������� ������ ��������� ��������� �������� (���������� �����, ��� ��������� ����� ���������� ����������, ���������� ������)");
	//deliverySm.reports
	__reg_tc__("deliverySm.reports",
		"������ � �������� (������������� �������� � ������������� �����������)");
	__reg_tc__("deliverySm.reports.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
	__reg_tc__("deliverySm.reports.checkFields",
		"����� �������� ������������ �����");
	__reg_tc__("deliverySm.reports.multipleMessages",
		"������� ����� ��������� � �������� ����������� �� ��������� sms");
	//deliverySm.reports.deliveryReceipt
	__reg_tc__("deliverySm.reports.deliveryReceipt",
		"������������� ��������");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkAllowed",
		"�������� ������������� ��������� ������������� �������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� �� ��������� �������� ������������ pdu)");
	__reg_tc__("deliverySm.reports.deliveryReceipt.failureDeliveryReceipt",
		"������������� �������� �� ������ �� ������������ � ������ �������� �������� ������������� ���������");
	__reg_tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt",
		"������������� �������� �� ������ ��� ��������� ����� ���������� ������������ � ������ ������� validity period");
	__reg_tc__("deliverySm.reports.deliveryReceipt.recvTimeChecks",
		"������������� �������� �������� � ������ ��������� �������� ������������ pdu");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkStatus",
		"���������� � ������� ������������ pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkText",
		"����� ��������� ������������� ���������� SC");
	//deliverySm.reports.intermediateNotification
	__reg_tc__("deliverySm.reports.intermediateNotification",
		"������������� �����������");
	__reg_tc__("deliverySm.reports.intermediateNotification.checkAllowed",
		"�������� ������������� ��������� ������������� ����������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� ����� ������ ������������� ������� ��������)");
	__reg_tc__("deliverySm.reports.intermediateNotification.noRescheduling",
		"������������� ����������� �� ������������ � ������, ���� sms �� ���� ������������");
	__reg_tc__("deliverySm.reports.intermediateNotification.recvTimeChecks",
		"����� �������� ������������� ����������� ������������ ������� ������ ������������� ������� �������� ������������ pdu");
	__reg_tc__("deliverySm.reports.intermediateNotification.checkStatus",
		"���������� � ������� pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("deliverySm.reports.intermediateNotification.checkText",
		"����� ��������� ������������� ���������� SC");
	//deliverySm.smeAck
	__reg_tc__("deliverySm.smeAck",
		"�������� ��������� �� ���������� sme SC (�������� ��������, db sme � �.�.)");
	__reg_tc__("deliverySm.smeAck.checkAllowed",
		"�������� ������������� ��������� ���������");
	__reg_tc__("deliverySm.smeAck.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ���������� sme SC)");
	__reg_tc__("deliverySm.smeAck.checkFields",
		"����� �������� ������������ �����");
	__reg_tc__("deliverySm.smeAck.recvTimeChecks",
		"���������� ����� ��������� ���������");
	//deliverySm.resp
	__reg_tc__("deliverySm.resp", "�������� deliver_sm_resp pdu");
	__reg_tc__("deliverySm.resp.sync",
		"�������� ����������� deliver_sm_resp pdu");
	__reg_tc__("deliverySm.resp.async",
		"�������� ������������ deliver_sm_resp pdu");
	__reg_tc__("deliverySm.resp.delay",
		"�������� deliver_sm_resp pdu � ��������� ������� sme timeout");
	__reg_tc__("deliverySm.resp.sendOk",
		"��������� ������� �������������� ��������� ����������� deliver_sm");
	//deliverySm.resp.sendRetry
	__reg_tc__("deliverySm.resp.sendRetry",
		"��������� ������� ��� ������������ ��������� �������� deliver_sm");
	__reg_tc__("deliverySm.resp.sendRetry.notSend",
		"�� ��������� �������, �������� ����������� deliver_sm");
	__reg_tc__("deliverySm.resp.sendRetry.tempAppError",
		"��������� ������ ESME_RX_T_APPN (��������� ������ �� ������� sme, ������ �� ��������� ��������)");
	__reg_tc__("deliverySm.resp.sendRetry.msgQueueFull",
		"��������� ������ ESME_RMSGQFUL (������������ ������� ������� sme)");
	__reg_tc__("deliverySm.resp.sendRetry.invalidSequenceNumber",
		"��������� ������� � ������������ sequence_number");
	__reg_tc__("deliverySm.resp.sendRetry.sendAfterSmeTimeout",
		"��������� ������� ����� sme timeout");
	//deliverySm.resp.sendError
	__reg_tc__("deliverySm.resp.sendError",
		"��������� ������� ��� ����������� ��������� �������� deliver_sm");
	__reg_tc__("deliverySm.resp.sendError.standardError",
		"������� �� ����������� ����� ������ (�������� 0x1-0x10f)");
	__reg_tc__("deliverySm.resp.sendError.reservedError",
		"������� � ����������������� ����� ������ (��������� 0x110-0x3ff � 0x400-0x4ff)");
	__reg_tc__("deliverySm.resp.sendError.outRangeError",
		"������� � ����� ������ ��� ��������� ������������� ������������� SMPP (>0x500)");
	__reg_tc__("deliverySm.resp.sendError.permanentAppError",
		"������� � ����� ������ ESME_RX_P_APPN (������������ ������ �� ������� sme, ����� �� ���� ����������� ���������)");
}

void sendInvalidPduTc()
{
	__reg_tc__("sendInvalidPdu",
		"�������� ������������ pdu");
	__reg_tc__("sendInvalidPdu.sync",
		"���������� �������� pdu");
	__reg_tc__("sendInvalidPdu.async",
		"����������� �������� pdu");
	__reg_tc__("sendInvalidPdu.request",
		"������� ������������ �����");
	__reg_tc__("sendInvalidPdu.response",
		"������� ������������ �����");
	__reg_tc__("sendInvalidPdu.invalidCommandId",
		"������������ command_id");
}

void processGenericNackTc()
{
	__reg_tc__("processGenericNack", "��������� generic_nack pdu");
	__reg_tc__("processGenericNack.sync",
		"Exception ��� ������������ ���������� ��������");
	__reg_tc__("processGenericNack.async",
		"��������� generic_nack pdu ��� ��� ������������ ����������� ��������");
	__reg_tc__("processGenericNack.checkHeader",
		"���������� �������� ����� ������ (command_length, command_id, sequence_number)");
	__reg_tc__("processGenericNack.checkTime",
		"���������� ����� ��������� pdu");
	__reg_tc__("processGenericNack.checkStatusInvalidCommandLength",
		"���� ��� ������ ESME_RINVCMDLEN � ���� command_status, �� ��� pdu ����� ���������, � ����� ������������");
	__reg_tc__("processGenericNack.checkStatusInvalidCommandId",
		"���� ��� ������ ESME_RINVCMDID � ���� command_status, �� ��� pdu ������������� ����� �����������");
	__reg_tc__("processGenericNack.checkStatusOther",
		"������ ���� ������ ������������� ������������");
}

void checkMissingPduTc()
{
	__reg_tc__("checkMissingPdu",
		"��� ������������ pdu ������������ � �� ��������");
	//response
	__reg_tc__("checkMissingPdu.response",
		"�� ��� �������� �������� �������� (�� ��������) � ������������� ��������� � ������� �������� ��������");
	__reg_tc__("checkMissingPdu.delivery",
		"��������� ������������ (�� ��������) � ��������� � schedule_delivery_time �� validity_period � ������������� ���������");
	__reg_tc__("checkMissingPdu.deliveryReceipt",
		"������������ �������� ������������ (�� ��������) � ��������� � ������� �������� ������������� ��������� � ������� ������������� ������� ���������� ��������� ��� SC");
	__reg_tc__("checkMissingPdu.intermediateNotification",
		"������������� ����������� ������������ � ��������� � submit_time �� validity_period");
	__reg_tc__("checkMissingPdu.smeAcknoledgement",
		"�������� ��������� �� ����������� sme ������������ � ��������� � ������� �������� ������������� ��������� � ������� ������������� ������� ���������� ��������� ��� SC");
	__reg_tc__("checkMissingPdu.genericNack",
		"GenericNack pdu ������������ (�� ��������) � ������������� ��������� � ������� �������� ������������ pdu");
}

void protocolError()
{
	__reg_tc__("protocolError", "������������ ��������� ��������");
	//invalidBind
	__reg_tc__("protocolError.invalidBind",
		"�������� ������������ bind pdu (����� ����� ������������ ���������� � SC)");
	__reg_tc__("protocolError.invalidBind.smallerSize1",
		"���� ������ pdu ������ 16 ���� (������ ������), SC ��������� ����������");
	__reg_tc__("protocolError.invalidBind.smallerSize2",
		"���� ������ pdu ������ �����������, SC ��������� ����������");
	__reg_tc__("protocolError.invalidBind.greaterSize1",
		"���� ������ pdu ������ �����������, �� ������ 64kb, SC ��������� ���������� ����� ��������");
	__reg_tc__("protocolError.invalidBind.greaterSize2",
		"���� ������ pdu ������ 100kb, SC ��������� ����������");
	__reg_tc__("protocolError.invalidBind.allowedCommandId",
		"���� ����� command_id ����������� ��� sme, �� �� bind, SC ���������� generic_nack � command_status = ESME_RINVBNDSTS");
	__reg_tc__("protocolError.invalidBind.notAllowedCommandId",
		"���� ����� command_id ������������� ��� sme (pdu ������������ ������������� SC), SC ���������� generic_nack � command_status = ESME_RINVCMDID");
	__reg_tc__("protocolError.invalidBind.nonExistentCommandId",
		"���� ����� �������������� � ������������ command_id, SC ���������� generic_nack � command_status = ESME_RINVBNDSTS");
	//corruptedPdu
	__reg_tc__("protocolError.invalidPdu",
		"�������� ������������ pdu ����� ������������ ���������� � ��������� bind");
	__reg_tc__("protocolError.invalidPdu.smallerSize1",
		"���� ������ pdu ������ 16 ���� (������ ������), SC ��������� ����������");
	__reg_tc__("protocolError.invalidPdu.smallerSize2",
		"���� ������ pdu ������ �����������, SC ��������� ����������");
	__reg_tc__("protocolError.invalidPdu.greaterSize1",
		"���� ������ pdu ������ �����������, �� ������ 64kb, SC ��������� ���������� ����� ��������");
	__reg_tc__("protocolError.invalidPdu.greaterSize2",
		"���� ������ pdu ������ 100kb, SC ��������� ����������");
	__reg_tc__("protocolError.invalidPdu.notAllowedCommandId",
		"���� ����� command_id ������������� ��� sme (pdu ������������ ������������� SC), SC ���������� generic_nack � command_status = ESME_RINVCMDID");
	__reg_tc__("protocolError.invalidPdu.nonExistentCommandId",
		"���� ����� �������������� � ������������ command_id, SC ���������� generic_nack � command_status = ESME_RINVCMDID");
	//submitAfterUnbind
	__reg_tc__("protocolError.submitAfterUnbind",
		"Pdu ������������ ����� unbind ������������ SC");
	__reg_tc__("protocolError.submitAfterUnbind.allowedCommandId",
		"���� ����� command_id ����������� ��� sme, �� �� bind, SC ���������� generic_nack � command_status = ESME_RINVBNDSTS");
	__reg_tc__("protocolError.submitAfterUnbind.notAllowedCommandId",
		"���� ����� command_id ������������� ��� sme (pdu ������������ ������������� SC), SC ���������� generic_nack � command_status = ESME_RINVCMDID");
	__reg_tc__("protocolError.submitAfterUnbind.nonExistentCommandId",
		"���� ����� �������������� � ������������ command_id, SC ���������� generic_nack � command_status = ESME_RINVBNDSTS");
	//other
	__reg_tc__("protocolError.equalSeqNum",
		"�������� ���������� submit_sm � ���������� sequence_number �� ������ �� ������ ��������� pdu");
	__reg_tc__("protocolError.nullPdu",
		"�������� pdu � ������� (NULL) ������");
}

void notImplementedTc()
{
	__reg_tc__("notImplemented", "�� ������������������ ���� �����");
	__reg_tc__("notImplemented.submitMulti", "�������� submit_multi pdu");
	__reg_tc__("notImplemented.processMultiResp", "��������� submit_multi_resp pdu");
	__reg_tc__("notImplemented.dataSm", "�������� data_sm pdu");
	__reg_tc__("notImplemented.processDataSm", "��������� data_sm pdu");
	__reg_tc__("notImplemented.processDataSmResp", "��������� data_sm_resp pdu");
	__reg_tc__("notImplemented.querySm", "�������� query_sm pdu");
	__reg_tc__("notImplemented.processQuerySmResp", "��������� query_sm_resp pdu");
	__reg_tc__("notImplemented.cancelSm", "�������� cancel_sm pdu");
	__reg_tc__("notImplemented.processCancelSmResp", "��������� cancel_sm_resp pdu");
	__reg_tc__("notImplemented.processAlertNotification", "��������� alert_notification pdu");
}

void allProtocolTc()
{
	bindTc();
	unbindTc();
	submitSmTc();
	replaceSmTc();
	deliverySmTc();


	sendInvalidPduTc();
	//receiver
	processGenericNackTc();
	//processIntermediateNotificationTc();
	//other
	checkMissingPduTc();
	protocolError();
	notImplementedTc();
}

public:
	SmppProtocolCheckList()
		: CheckList("���������� ��������������� ������������ ��������� SMPP", "smpp.chk")
	{
		allProtocolTc();
	}

protected:
	SmppProtocolCheckList(const char* name, const char* fileName)
		: CheckList(name, fileName)
	{
		allProtocolTc();
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_CHECK_LIST */

