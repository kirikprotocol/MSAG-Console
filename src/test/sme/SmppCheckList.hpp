#ifndef TEST_SME_SMPP_CHECK_LIST
#define TEST_SME_SMPP_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmppCheckList : public CheckList
{
public:
	SmppCheckList()
		: CheckList("���������� ��������������� ������������ BaseSme � ��������� SMPP", "smpp.chk")
	{
		//bindCorrectSme
		__reg_tc__("bindCorrectSme",
			"������������ IP ���������� � SC � �����������");
		__reg_tc__("bindCorrectSme.bindReceiver",
			"����������� �������� � ������� bind_receiver pdu");
		__reg_tc__("bindCorrectSme.bindTransmitter",
			"����������� ������������ � ������� bind_transmitter pdu");
		__reg_tc__("bindCorrectSme.bindTransceiver",
			"����������� ���������� � ������� bind_transceiver pdu");
		//bindIncorrectSme
		__reg_tc__("bindIncorrectSme",
			"������������ IP ���������� � SC � ����������� � ������������� �����������");
		__reg_tc__("bindIncorrectSme.smeNotRegistered",
			"�������� system_id �� ��������� � ������������ SC");
		__reg_tc__("bindIncorrectSme.unknownHost",
			"��������� ���������� � ����������� SC (����������� ����) ����������� ���������");
		__reg_tc__("bindIncorrectSme.invalidPort",
			"��������� ���������� � ����������� SC (������������ ����) ����������� ���������");
		//unbind
		__reg_tc__("unbind", "���������� ���������� � SC");
		//submitSm
		__reg_tc__("submitSm",
			"�������� submit_sm pdu � ����������� ���������� �����");
		__reg_tc__("submitSm.serviceTypeMarginal",
			"���������� �������� ����� ���� service_type");
		__reg_tc__("submitSm.waitTimePast",
			"����� schedule_delivery_time � ������� (�������� ��� ������ ���� ��������)");
		__reg_tc__("submitSm.validTimePast",
			"����� validity_period � ������� (��������� ������� �� ����� ����������)");
		__reg_tc__("submitSm.validTimeExceeded",
			"����� validity_period ������ ����������� ����� ���������� �������������� ��� SC");
		__reg_tc__("submitSm.waitTimeInvalid",
			"�������� schedule_delivery_time ��������� validity_period");
		__reg_tc__("submitSm.smLengthMarginal",
			"���������� �������� ����� ���� ���������");
		__reg_tc__("submitSm.checkMap",
			"������������� ��������� �������� MAP (��������� source_addr, dest_addr � user_message_reference), �� �������� SMPP ������������ ��������� ����������� � ������� �������� �������� ���� �� �����");
		__reg_tc__("submitSm.notReplace",
			"������������� ��������� (��������� source_addr, dest_addr � service_type) ��� ��������� �������������, ������������ � ������� ��������");
		__reg_tc__("submitSm.serviceTypeNotMatch",
			"��������� source_addr � dest_addr, ���������� service_type ��� ��������� ������������ � ������� ��������");
		__reg_tc__("submitSm.replaceEnrote",
			"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� �������������, ������������ � ������� ��������");
		__reg_tc__("submitSm.replaceReplacedEnrote",
			"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� ��� ����� ����������� � ������������ � ������� ��������");
		__reg_tc__("submitSm.replaceFinal",
			"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� ��� �������������");
		__reg_tc__("submitSm.replaceRepeatedDeliveryEnrote",
			"������������� ��������� (��������� source_addr, dest_addr � service_type) � ���������� �������������, ������������ � ������� ��������� ��������");
		__reg_tc__("submitSm.sync",
			"�������� ����������� submit_sm pdu");
		__reg_tc__("submitSm.async",
			"�������� ������������ submit_sm pdu");
		//submitSmAssert
		__reg_tc__("submitSmAssert",
			"���������� � �������� submit_sm pdu � ������������� ���������� �����");
		__reg_tc__("submitSmAssert.serviceTypeInvalid",
			"������������ �������� ����� ���� service_type");
		__reg_tc__("submitSmAssert.destAddrLengthInvalid",
			"������������ �������� ����� ���� ������ ����������");
		__reg_tc__("submitSmAssert.msgLengthInvalid",
			"������������ �������� ����� ������ ��������� short_message");
		//replaceSm
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
		__reg_tc__("processResp.checkDelivery",
			"��� ������� ���� ������ � ���� command_status, �������� deliver_sm �� ����������");
		__reg_tc__("processResp.checkDeliveryReceipt",
			"��� ������� ���� ������ � ���� command_status, ������������� �������� �� ����������");
		__reg_tc__("processResp.checkIntermediateNotification",
			"��� ������� ���� ������ � ���� command_status, ������������� ����������� �� ����������");
		__reg_tc__("processResp.checkCmdStatusOk",
			"��� ���������� ���� ������ � ���� command_status, ����������� ��� ������� ��� ���������� �������� ��������� (���� �������� ������ ���������, ���������� ������� � �.�.)");
		__reg_tc__("processResp.checkCmdStatusInvalidDestAddr",
			"���� ��� ������ ESME_RINVDSTADR � ���� command_status, �� �������� ������������� �� ����������");
		__reg_tc__("processResp.checkCmdStatusInvalidWaitTime",
			"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� ����� schedule_delivery_time ������������� ������ �����������");
		__reg_tc__("processResp.checkCmdStatusInvalidValidTime",
			"���� ��� ������ ESME_RINVEXPIRY � ���� command_status, �� ����� validity_period ������������� ������ �����������");
		//sendDeliverySmResp
		__reg_tc__("sendDeliverySmResp",
			"�������� deliver_sm_resp pdu");
		__reg_tc__("sendDeliverySmResp.notSend",
			"�� ��������� ������� ��� ������������ ��������� �������� deliver_sm �� ������� SC");
		__reg_tc__("sendDeliverySmResp.sendWithErrCode",
			"��������� ������� � ����� ������ ��� ������������ ��������� �������� deliver_sm �� ������� SC");
		__reg_tc__("sendDeliverySmResp.sendInvalidSequenceNumber",
			"��������� ������� � ������������ sequence_number");
		__reg_tc__("sendDeliverySmResp.sendOk",
			"��������� ������� �������������� ��������� ����������� deliver_sm");
		__reg_tc__("sendDeliverySmResp.sync",
			"�������� ����������� deliver_sm_resp pdu");
		__reg_tc__("sendDeliverySmResp.async",
			"�������� ������������ deliver_sm_resp pdu");
		//processDeliverySm
		__reg_tc__("processDeliverySm", "��������� deliver_sm pdu");
		__reg_tc__("processDeliverySm.checkFields",
			"����� �������� ����� ������ � ��������� ������ �����");
		//normalSms
		__reg_tc__("processDeliverySm.normalSms",
			"������� sms �� ���������� ������������");
		__reg_tc__("processDeliverySm.normalSms.checkAllowed",
			"�������� ������������� ��������� sms");
		__reg_tc__("processDeliverySm.normalSms.checkRoute",
			"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ����������)");
		__reg_tc__("processDeliverySm.normalSms.checkMandatoryFields",
			"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm) pdu");
		__reg_tc__("processDeliverySm.normalSms.checkOptionalFields",
			"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm) pdu");
		__reg_tc__("processDeliverySm.normalSms.scheduleChecks",
			"���������� ������ ��������� ��������� �������� (���������� �����, ��� ��������� ����� ���������� ����������, ���������� ������)");
		__reg_tc__("processDeliverySm.normalSms.checkDeliveryReceipt",
			"������������� �������� ������������ �� ������ ����� sms");
		//deliveryReceipt
		__reg_tc__("processDeliverySm.deliveryReceipt",
			"������������� ��������");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkAllowed",
			"�������� ������������� ��������� ������������� ��������");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkRoute",
			"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkFields",
			"����� �������� ������������ �����");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkStatus",
			"���������� � ������������ pdu �������� ����������");
		__reg_tc__("processDeliverySm.deliveryReceipt.scheduleChecks",
			"���������� ������ ��������� ��������� �������� (���������� �����, ��� ��������� ����� ���������� ����������, ���������� ������)");
		//intermediateNotification
		__reg_tc__("processDeliverySm.intermediateNotification",
			"������������� �����������");
		__reg_tc__("processDeliverySm.intermediateNotification.checkAllowed",
			"�������� ������������� ��������� ������������� ������������� �����������");
		__reg_tc__("processDeliverySm.intermediateNotification.checkRoute",
			"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
		__reg_tc__("processDeliverySm.intermediateNotification.checkFields",
			"����� �������� ������������ �����");
		__reg_tc__("processDeliverySm.intermediateNotification.checkStatus",
			"���������� � ��������� ������������ pdu �������� ����������");
		__reg_tc__("processDeliverySm.intermediateNotification.checkTime",
			"���������� ����� ��������");
		//checkMissingPdu
		__reg_tc__("checkMissingPdu",
			"��� ������������ pdu ������������ � �� ��������");
		//response
		__reg_tc__("checkMissingPdu.response",
			"�� ��� �������� �������� �������� (�� ��������) � ������������� ��������� � ������� �������� ��������");
		//delivery
		__reg_tc__("checkMissingPdu.delivery",
			"��������� ������������ (�� ��������) � ��������� � schedule_delivery_time �� validity_period � ������������� ���������");
		__reg_tc__("checkMissingPdu.delivery.waitTime",
			"��������� � ���������� ��������� � ��������� ����������� ������������ (�� ��������) � ������ ������� schedule_delivery_time");
		__reg_tc__("checkMissingPdu.delivery.validTime",
			"��� ����������� ���������� ��������� �������� ��������� ������������ ������ �� ������ ������� validity_period");
		//deliveryReceipt
		__reg_tc__("checkMissingPdu.deliveryReceipt",
			"������������ �������� ������������ (�� ��������) � ��������� � schedule_delivery_time �� validity_period � ������������� ���������");
		__reg_tc__("checkMissingPdu.deliveryReceipt.waitTime",
			"������������ �������� ��� ��������� ���������� ��������� ������������ (�� ��������) � ������ ������� schedule_delivery_time");
		__reg_tc__("checkMissingPdu.deliveryReceipt.validTime",
			"��� ����������� ���������� ������������ �������� (����������) ��������� ������������ ����������� � ������ ������� validity_period");
		//intermediateNotification
		__reg_tc__("checkMissingPdu.intermediateNotification",
			"������������� ����������� ������������ � ��������� � submit_time �� validity_period � ������������� ���������");
		__reg_tc__("checkMissingPdu.intermediateNotification.waitTime",
			"��� ��������� ���������� ��������� �� ������ ������� schedule_delivery_time ������ ������������ ������������� �����������");
		__reg_tc__("checkMissingPdu.intermediateNotification.validTime",
			"��� ����������� ���������� ������������� ����������� ������������ ������ �� ������� ������� validity_period");
		//not implemented
		__reg_tc__("processGenericNack", "��������� generic_nack pdu");
		__reg_tc__("submitMulti", "�������� submit_multi pdu");
		__reg_tc__("processMultiResp", "��������� submit_multi_resp pdu");
		__reg_tc__("dataSm", "�������� data_sm pdu");
		__reg_tc__("processDataSm", "��������� data_sm pdu");
		__reg_tc__("processDataSmResp", "��������� data_sm_resp pdu");
		__reg_tc__("querySm", "�������� query_sm pdu");
		__reg_tc__("processQuerySmResp", "��������� query_sm_resp pdu");
		__reg_tc__("cancelSm", "�������� cancel_sm pdu");
		__reg_tc__("processCancelSmResp", "��������� cancel_sm_resp pdu");
		__reg_tc__("processAlertNotification", "��������� alert_notification pdu");
	}
};

}
}
}

#endif /* TEST_SME_SMPP_CHECK_LIST */

