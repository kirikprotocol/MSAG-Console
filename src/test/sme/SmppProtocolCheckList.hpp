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
	/*
	__reg_tc__("bindCorrectSme.bindReceiver",
		"����������� �������� � ������� bind_receiver pdu");
	__reg_tc__("bindCorrectSme.bindTransmitter",
		"����������� ������������ � ������� bind_transmitter pdu");
	*/
	__reg_tc__("bindCorrectSme.bindTransceiver",
		"����������� ���������� � ������� bind_transceiver pdu");
}

void bindIncorrectSmeTc()
{
	__reg_tc__("bindIncorrectSme",
		"������������ IP ���������� � SC � ����������� � ������������� �����������");
	__reg_tc__("bindIncorrectSme.smeNotRegistered",
		"�������� system_id �� ��������� � ������������ SC");
	__reg_tc__("bindIncorrectSme.unknownHost",
		"��������� ���������� � ����������� SC (����������� ����) ����������� ���������");
	__reg_tc__("bindIncorrectSme.invalidPort",
		"��������� ���������� � ����������� SC (������������ ����) ����������� ���������");
}

void unbindTc()
{
	__reg_tc__("unbind", "���������� ���������� � SC");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "�������� submit_sm pdu");
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

void processRespTc()
{
	//processSubmitSmResp
	__reg_tc__("processSubmitSmResp",
		"��������� submit_sm_resp pdu");
	__reg_tc__("processSubmitSmResp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id)");
	__reg_tc__("processSubmitSmResp.sync",
		"��������� submit_sm_resp pdu ��� ���������� submit_sm ��������");
	__reg_tc__("processSubmitSmResp.async",
		"��������� submit_sm_resp pdu ��� ����������� submit_sm ��������");
	//processReplaceSmResp
	__reg_tc__("processReplaceSmResp",
		"��������� replace_sm_resp pdu");
	__reg_tc__("processReplaceSmResp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id)");
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
	__reg_tc__("processResp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void sendDeliverySmRespTc()
{
	__reg_tc__("sendDeliverySmResp", "�������� deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.sync",
		"�������� ����������� deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.async",
		"�������� ������������ deliver_sm_resp pdu");
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

void processDeliveryReceiptTc()
{
	__reg_tc__("processDeliverySm.deliveryReceipt",
		"������������� ��������");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkAllowed",
		"�������� ������������� ��������� ������������� ��������");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkFields",
		"����� �������� ������������ �����");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkFieldsExt",
		"�������� ������������ ����� � � ������ ����������� ����� (none, final, failure) ������������� � �������� ������� �� ������ �������� ������������ pdu");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkState",
		"���������� � ������� ������������ pdu �������� ����������");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkErrorCode",
		"���������� � ���� ������ � ������ ���������� pdu �������� ����������");
	__reg_tc__("processDeliverySm.deliveryReceipt.deliveryRescheduled",
		"������������� �������� ������������ ����� ����� ��������� ��������� �������� ������������� ���������");
	__reg_tc__("processDeliverySm.deliveryReceipt.failureDeliveryReceipt",
		"������������� �������� �� ������ �� ������������ � ������ �������� �������� ������������� ���������");
	__reg_tc__("processDeliverySm.deliveryReceipt.expiredDeliveryReceipt",
		"������������� �������� �� ������ ��� ��������� ����� ���������� ������������ � ������ ������� validity period");
	__reg_tc__("processDeliverySm.deliveryReceipt.scheduleChecks",
		"���������� ������ ��������� ��������� �������� (���������� �����, ��� ��������� ����� ���������� ����������, ���������� ������)");
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
}

void notImplementedTc()
{
	__reg_tc__("notImplemented", "�� ������������������ ���� �����");
	__reg_tc__("notImplemented.processGenericNack", "��������� generic_nack pdu");
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
	//receiver
	processRespTc();
	processDeliverySmTc();
	processNormalSmsTc();
	processDeliveryReceiptTc();
	processSmeAckTc();
	//processIntermediateNotificationTc();
	//other
	checkMissingPduTc();
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

