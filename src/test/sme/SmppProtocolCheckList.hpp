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

void bindCorrectSmeTc()
{
	__reg_tc__("bindCorrectSme",
		"������������ IP ���������� � SC � �����������");
	__reg_tc__("bindCorrectSme.bindReceiver",
		"����������� �������� � ������� bind_receiver pdu");
	__reg_tc__("bindCorrectSme.bindTransmitter",
		"����������� ������������ � ������� bind_transmitter pdu");
	__reg_tc__("bindCorrectSme.bindTransceiver",
		"����������� ���������� � ������� bind_transceiver pdu");
	__reg_tc__("bindCorrectSme.checkCommandStatus",
		"� bind �������� �������� ���� command_status ����� ESME_ROK");
	__reg_tc__("bindCorrectSme.checkInterfaceVersion",
		"� bind �������� �������� ���� sc_interface_version ����� 0x34");
}

void bindIncorrectSmeTc()
{
	__reg_tc__("bindIncorrectSme",
		"������������ IP ���������� � SC � ����������� � ������������� �����������");
	__reg_tc__("bindIncorrectSme.systemIdNotRegistered",
		"�������� system_id �� ��������� � ������������ SC");
	__reg_tc__("bindIncorrectSme.invalidPassword",
		"������������ ������");
	__reg_tc__("bindIncorrectSme.unknownHost",
		"��������� ���������� � ����������� SC (����������� ����) ����������� ���������");
	__reg_tc__("bindIncorrectSme.invalidPort",
		"��������� ���������� � ����������� SC (������������ ����) ����������� ���������");
	__reg_tc__("bindIncorrectSme.invalidSystemIdLength",
		"����� ���� system_id � bind pdu ������ ����������� ����������");
	__reg_tc__("bindIncorrectSme.invalidPasswordLength",
		"����� ���� password � bind pdu ������ ����������� ����������");
	__reg_tc__("bindIncorrectSme.invalidSystemTypeLength",
		"����� ���� system_type � bind pdu ������ ����������� ����������");
	__reg_tc__("bindIncorrectSme.invalidAddressRangeLength",
		"����� ���� address_range � bind pdu ������ ����������� ����������");
}

void unbindTc()
{
	__reg_tc__("unbind", "���������� ���������� � SC");
	__reg_tc__("unbind.checkCommandStatus",
		"� unbind �������� �������� ���� command_status ����� ESME_ROK");
	__reg_tc__("unbind.checkSocketClose",
		"����� �������� unbind �������� SC ��������� ����������");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "�������� submit_sm pdu");
	__reg_tc__("submitSm.sync", "�������� ����������� submit_sm pdu");
	__reg_tc__("submitSm.async", "�������� ������������ submit_sm pdu");
	__reg_tc__("submitSm.receiver",
		"���� sme ���������������� ��� receiver, �� SmppSession::submit() ������ ����������");
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
		"��������� source_addr � dest_addr, ���������� service_type ��� ��������� ������������ � ������� ��������");
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
}

void replaceSmTc()
{
	__reg_tc__("replaceSm",
		"�������� replace_sm pdu � ����������� ���������� �����");
	__reg_tc__("replaceSm.waitTimePast",
		"����� schedule_delivery_time � ������� (�������� ��� ������ ���� ��������)");
	__reg_tc__("replaceSm.validTimePast",
		"����� validity_period � ������� (��������� ������� �� ����� ����������)");
	__reg_tc__("replaceSm.validTimeExceeded",
		"����� validity_period ������ ����������� ����� ���������� �������������� ��� SC");
	__reg_tc__("replaceSm.waitTimeInvalid",
		"�������� schedule_delivery_time ��������� validity_period");
	__reg_tc__("replaceSm.smLengthMarginal",
		"���������� �������� ����� ���� ��������� short_message");
	__reg_tc__("replaceSm.msgIdNotExist",
		"���� message_id ������ �������������� ���������");
	__reg_tc__("replaceSm.replaceReplacedEnrote",
		"��������� ��� ����� ����������� ��������� � ������������ � ������� ��������");
	__reg_tc__("replaceSm.replaceFinal",
		"��������� ��� ������������� ���������");
	__reg_tc__("replaceSm.replaceRepeatedDeliveryEnrote",
	   "��������� ��������� ������������ � ������� ��������� ��������");
	__reg_tc__("replaceSm.sync",
		"�������� ����������� replace_sm pdu");
	__reg_tc__("replaceSm.async",
		"�������� ������������ replace_sm pdu");
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

void processRespTc()
{
	//processSubmitSmResp
	__reg_tc__("processSubmitSmResp",
		"��������� submit_sm_resp pdu");
	__reg_tc__("processSubmitSmResp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("processSubmitSmResp.sync",
		"��������� submit_sm_resp pdu ��� ���������� submit_sm ��������");
	__reg_tc__("processSubmitSmResp.async",
		"��������� submit_sm_resp pdu ��� ����������� submit_sm ��������");
	//processReplaceSmResp
	__reg_tc__("processReplaceSmResp",
		"��������� replace_sm_resp pdu");
	__reg_tc__("processReplaceSmResp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("processReplaceSmResp.sync",
		"��������� replace_sm_resp pdu ��� ���������� replace_sm ��������");
	__reg_tc__("processReplaceSmResp.async",
		"��������� replace_sm_resp pdu ��� ����������� replace_sm ��������");
	//processResp
	__reg_tc__("processResp",
		"����� �������� ��� submit_sm_resp, replace_sm_resp � data_sm_resp");
	__reg_tc__("processResp.checkHeader",
		"���������� �������� ����� ������ �������� (sequence_number)");
	__reg_tc__("processResp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("processResp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("processResp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status, ����������� ��� ������� ��� ���������� �������� ��������� (���� �������� ������ ���������, ���������� ������� � �.�.)");
	__reg_tc__("processResp.checkCmdStatusInvalidDestAddr",
		"���� ��� ������ ESME_RINVDSTADR � ���� command_status, �� �������� ������������� �� ����������");
	__reg_tc__("processResp.checkCmdStatusInvalidWaitTime",
		"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� ����� schedule_delivery_time ������������� ������ �����������");
	__reg_tc__("processResp.checkCmdStatusInvalidValidTime",
		"���� ��� ������ ESME_RINVEXPIRY � ���� command_status, �� ����� validity_period ������������� ������ �����������");
	__reg_tc__("processResp.checkCmdStatusInvalidDataCoding",
		"���� ��� ������ ESME_RINVDCS � ���� command_status, �� ��������� ��������� ������������� ������ �����������");
	__reg_tc__("processResp.checkCmdStatusInvalidServiceType",
		"���� ��� ������ ESME_RINVSERTYP � ���� command_status, �� ����� ���� service_type ������������� ������ �����������");
	__reg_tc__("processResp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� ����� ����������� ������������� �� ������������� address range ��� ������ sme � ������������ sme.xml SC");
	__reg_tc__("processResp.checkCmdStatusSystemError",
		"���� ��� ������ ESME_RSYSERR � ���� command_status, �� �� ������� SC ������������� �������� ������������ ������ (transaction rollback ��� ���������� ���������)");
	__reg_tc__("processResp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ INVALID_BIND_STATUS � ���� command_status, �� ������������� sme ������������������ ��� receiver ���������� �������� (submit_sm, replace_sm, cancel_sm � �.�.)");
	__reg_tc__("processResp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
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

void sendDeliverySmRespTc()
{
	__reg_tc__("sendDeliverySmResp", "�������� deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.sync",
		"�������� ����������� deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.async",
		"�������� ������������ deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.delay",
		"�������� deliver_sm_resp pdu � ��������� ������� sme timeout");
	__reg_tc__("sendDeliverySmResp.sendOk",
		"��������� ������� �������������� ��������� ����������� deliver_sm");
	//sendDeliverySmResp.sendRetry
	__reg_tc__("sendDeliverySmResp.sendRetry",
		"��������� ������� ��� ������������ ��������� �������� deliver_sm");
	__reg_tc__("sendDeliverySmResp.sendRetry.notSend",
		"�� ��������� �������, �������� ����������� deliver_sm");
	__reg_tc__("sendDeliverySmResp.sendRetry.tempAppError",
		"��������� ������ ESME_RX_T_APPN (��������� ������ �� ������� sme, ������ �� ��������� ��������)");
	__reg_tc__("sendDeliverySmResp.sendRetry.msgQueueFull",
		"��������� ������ ESME_RMSGQFUL (������������ ������� ������� sme)");
	__reg_tc__("sendDeliverySmResp.sendRetry.invalidSequenceNumber",
		"��������� ������� � ������������ sequence_number");
	__reg_tc__("sendDeliverySmResp.sendRetry.sendAfterSmeTimeout",
		"��������� ������� ����� sme timeout");
	//sendDeliverySmResp.sendError
	__reg_tc__("sendDeliverySmResp.sendError",
		"��������� ������� ��� ����������� ��������� �������� deliver_sm");
	__reg_tc__("sendDeliverySmResp.sendError.standardError",
		"������� �� ����������� ����� ������ (�������� 0x1-0x10f)");
	__reg_tc__("sendDeliverySmResp.sendError.reservedError",
		"������� � ����������������� ����� ������ (��������� 0x110-0x3ff � 0x400-0x4ff)");
	__reg_tc__("sendDeliverySmResp.sendError.outRangeError",
		"������� � ����� ������ ��� ��������� ������������� ������������� SMPP (>0x500)");
	__reg_tc__("sendDeliverySmResp.sendError.permanentAppError",
		"������� � ����� ������ ESME_RX_P_APPN (������������ ������ �� ������� sme, ����� �� ���� ����������� ���������)");
}

void processDeliverySmTc()
{
	__reg_tc__("processDeliverySm", "��������� deliver_sm pdu");
	__reg_tc__("processDeliverySm.checkFields",
		"����� �������� ����� ������ � ��������� ������ �����");
}

void processNormalSmsTc()
{
	__reg_tc__("processDeliverySm.normalSms",
		"������� sms �� ���������� ������������");
	__reg_tc__("processDeliverySm.normalSms.checkAllowed",
		"�������� ������������� ��������� sms");
	__reg_tc__("processDeliverySm.normalSms.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ����������)");
	__reg_tc__("processDeliverySm.normalSms.checkMandatoryFields",
		"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm) pdu");
	__reg_tc__("processDeliverySm.normalSms.checkDataCoding",
		"��������� ������ sms ������������� ���������� ������� ����������");
	__reg_tc__("processDeliverySm.normalSms.checkTextEqualDataCoding",
		"��� ���������� ��������� � �������� ����������� � ���������� ����� sms ���������");
	__reg_tc__("processDeliverySm.normalSms.checkTextDiffDataCoding",
		"��� ��������� ��������� � �������� ����������� � ���������� ����� sms ��������� ������������� �� ����� ��������� � ������");
	__reg_tc__("processDeliverySm.normalSms.checkOptionalFields",
		"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm) pdu");
	__reg_tc__("processDeliverySm.normalSms.scheduleChecks",
		"���������� ������ ��������� ��������� �������� (���������� �����, ��� ��������� ����� ���������� ����������, ���������� ������)");
}

void processDeliveryReportTc()
{
	__reg_tc__("processDeliverySm.deliveryReport",
		"������ � �������� (������������� �������� � ������������� �����������)");
	__reg_tc__("processDeliverySm.deliveryReport.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
	__reg_tc__("processDeliverySm.deliveryReport.checkFields",
		"����� �������� ������������ �����");
	__reg_tc__("processDeliverySm.deliveryReport.multipleMessages",
		"������� ����� ��������� � �������� ����������� �� ��������� sms");
	//deliveryReceipt
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt",
		"������������� ��������");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkAllowed",
		"�������� ������������� ��������� ������������� �������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� �� ��������� �������� ������������ pdu)");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.failureDeliveryReceipt",
		"������������� �������� �� ������ �� ������������ � ������ �������� �������� ������������� ���������");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.expiredDeliveryReceipt",
		"������������� �������� �� ������ ��� ��������� ����� ���������� ������������ � ������ ������� validity period");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.recvTimeChecks",
		"������������� �������� �������� � ������ ��������� �������� ������������ pdu");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkStatus",
		"���������� � ������� ������������ pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkText",
		"����� ��������� ������������� ���������� SC");
	//intermediateNotification
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification",
		"������������� �����������");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.checkAllowed",
		"�������� ������������� ��������� ������������� ����������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� ����� ������ ������������� ������� ��������)");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.noRescheduling",
		"������������� ����������� �� ������������ � ������, ���� sms �� ���� ������������");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.recvTimeChecks",
		"����� �������� ������������� ����������� ������������ ������� ������ ������������� ������� �������� ������������ pdu");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.checkStatus",
		"���������� � ������� pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.checkText",
		"����� ��������� ������������� ���������� SC");
}

void processSmeAckTc()
{
	__reg_tc__("processDeliverySm.smeAck",
		"�������� ��������� �� ���������� sme SC (�������� ��������, db sme � �.�.)");
	__reg_tc__("processDeliverySm.smeAck.checkAllowed",
		"�������� ������������� ��������� ���������");
	__reg_tc__("processDeliverySm.smeAck.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ���������� sme SC)");
	__reg_tc__("processDeliverySm.smeAck.checkFields",
		"����� �������� ������������ �����");
	__reg_tc__("processDeliverySm.smeAck.recvTimeChecks",
		"���������� ����� ��������� ���������");
}

void processIntermediateNotificationTc()
{
	__reg_tc__("processDeliverySm.intermediateNotification",
		"������������� �����������");
	__reg_tc__("processDeliverySm.intermediateNotification.checkAllowed",
		"�������� ������������� ��������� ������������� ������������� �����������");
	__reg_tc__("processDeliverySm.intermediateNotification.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
	__reg_tc__("processDeliverySm.intermediateNotification.checkFields",
		"����� �������� ������������ �����");
	/*
	__reg_tc__("processDeliverySm.intermediateNotification.checkProfile",
		"�������� ������������ ���������� �������");
	__reg_tc__("processDeliverySm.intermediateNotification.checkStatus",
		"���������� � ��������� ������������ pdu �������� ����������");
	*/
	__reg_tc__("processDeliverySm.intermediateNotification.checkTime",
		"���������� ����� ��������");
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
	//transmitter
	bindCorrectSmeTc();
	bindIncorrectSmeTc();
	unbindTc();
	submitSmTc();
	replaceSmTc();
	sendDeliverySmRespTc();
	sendInvalidPduTc();
	//receiver
	processRespTc();
	processDeliverySmTc();
	processNormalSmsTc();
	processDeliveryReportTc();
	processSmeAckTc();
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

