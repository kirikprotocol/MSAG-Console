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
		"������������ IP ���������� � SC � bind ���� �����");
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
	__reg_tc__("bind.incorrect.secondBind",
		"�������� ���������� bind ��������");
	__reg_tc__("bind.incorrect.systemIdNotRegistered",
		"�������� system_id �� ��������� � ������������ SC");
	__reg_tc__("bind.incorrect.invalidPassword",
		"������������ ������");
	__reg_tc__("bind.incorrect.unknownHost",
		"��������� ���������� � ����������� SC (����������� ����) ����������� ���������");
	__reg_tc__("bind.incorrect.invalidPort",
		"��������� ���������� � ����������� SC (������������ ����) ����������� ���������");
	__reg_tc__("bind.incorrect.invalidSystemIdLength",
		"����� ���� system_id � bind pdu ������ ����������� ���������� (SmppSession::connect() ������� exception)");
	__reg_tc__("bind.incorrect.invalidPasswordLength",
		"����� ���� password � bind pdu ������ ����������� ���������� (SmppSession::connect() ������� exception)");
	__reg_tc__("bind.incorrect.invalidSystemTypeLength",
		"����� ���� system_type � bind pdu ������ ����������� ���������� (SmppSession::connect() ������� exception)");
	__reg_tc__("bind.incorrect.invalidAddressRangeLength",
		"����� ���� address_range � bind pdu ������ ����������� ���������� (SmppSession::connect() ������� exception)");
	//bind.resp
	__reg_tc__("bind.resp",
		"�� bind ������� SC ���������� ��������������� bind �������");
	__reg_tc__("bind.resp.receiver",
		"�� bind_receiver ������� SC ���������� bind_receiver_resp �������");
	__reg_tc__("bind.resp.transmitter",
		"�� bind_transmitter ������� SC ���������� bind_transmitter_resp �������");
	__reg_tc__("bind.resp.transceiver",
		"�� bind_transceiver ������� SC ���������� bind_transceiver_resp �������");
	__reg_tc__("bind.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("bind.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("bind.resp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, sequence_number)");
	__reg_tc__("bind.resp.checkFields",
		"�������� sc_interface_version = 0x34 (system_id �� �����������)");
	__reg_tc__("bind.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status, ����������� ��� ������� ��� ���������� ����������� sme (���������� ������, ������������������ system_id � �.�.)");
	__reg_tc__("bind.resp.checkCmdStatusAlreadyBound",
		"���� ��� ������ ESME_RALYBND � ���� command_status, �� ������������� sme �������� ������������������ ��������");
	__reg_tc__("bind.resp.checkCmdStatusInvalidPassword",
		"���� ��� ������ ESME_RINVPASWD � ���� command_status, �� ������������� ����� ���� password ��������� 8 ��������");
	__reg_tc__("bind.resp.checkCmdStatusInvalidSystemId",
		"���� ��� ������ ESME_RINVSYSID � ���� command_status, �� ������������� ����� ���� system_id ��������� 15 ��������");
	__reg_tc__("bind.resp.checkCmdStatusInvalidSystemType",
		"���� ��� ������ ESME_RINVSYSTYP � ���� command_status, �� ������������� ����� ���� system_type ��������� 12 ��������");
	__reg_tc__("bind.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void unbindTc()
{
	__reg_tc__("unbind", "���������� ���������� � SC � unbind ���� �����");
	__reg_tc__("unbind.resp",
		"�� unbind ������� SC ���������� unbind_resp �������");
	__reg_tc__("unbind.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("unbind.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("unbind.resp.checkPdu",
		"���������� �������� ����� �������� (command_length, sequence_number, command_status = ESME_ROK");
}

void enquireLinkTc()
{
	__reg_tc__("enquireLink",
		"�������� enquire_link ��������");
	__reg_tc__("enquireLink.receiver",
		"�������� enquire_link �������� � sme ������������������ ��� receiver");
	__reg_tc__("enquireLink.transmitter",
		"�������� enquire_link �������� � sme ������������������ ��� transmitter");
	__reg_tc__("enquireLink.transceiver",
		"�������� enquire_link �������� � sme ������������������ ��� transceiver");
	__reg_tc__("enquireLink.beforeBind",
		"�������� enquire_link �� �������� bind ��������");
	__reg_tc__("enquireLink.afterUnbind",
		"�������� enquire_link ����� �������� unbind ��������");
	//enquireLink.resp
	__reg_tc__("enquireLink.resp",
		"�� enquire_link ������� SC ���������� enquire_link_resp �������");
	__reg_tc__("enquireLink.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("enquireLink.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("enquireLink.resp.checkPdu",
		"���������� �������� ����� �������� (command_length, sequence_number, command_status = ESME_ROK ���������� �� bind �������");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "���� ����� ��� submit_sm");
	__reg_tc__("submitSm.sync",
		"�������� ����������� submit_sm ��������");
	__reg_tc__("submitSm.async",
		"�������� ������������ submit_sm ��������");
	__reg_tc__("submitSm.receiver",
		"�������� submit_sm �������� � sme ������������������ ��� receiver (SmppSession ������� exception)");
	__reg_tc__("submitSm.transmitter",
		"�������� submit_sm �������� � sme ������������������ ��� transmitter");
	__reg_tc__("submitSm.transceiver",
		"�������� submit_sm �������� � sme ������������������ ��� transceiver");
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
	__reg_tc__("replaceSm.receiver",
		"�������� replace_sm �������� � sme ������������������ ��� receiver (SmppSession ������� exception)");
	__reg_tc__("replaceSm.transmitter",
		"�������� replace_sm �������� � sme ������������������ ��� transmitter");
	__reg_tc__("replaceSm.transceiver",
		"�������� replace_sm �������� � sme ������������������ ��� transceiver");
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
	__reg_tc__("replaceSm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status ������� replace_sm ������������� �� �������� ������ (��������� ���������� � ��������� � ENROUTE ���������, ����� ����������� ��������� � �.�.)");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidWaitTime",
		"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� ����� schedule_delivery_time ������������� ������ �����������");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidValidTime",
		"���� ��� ������ ESME_RINVEXPIRY � ���� command_status, �� ����� validity_period ������������� ������ �����������");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� source_addr ������������� �� ������������� ������ ����������� ��������� � ��");
	__reg_tc__("replaceSm.resp.checkCmdStatusSystemError",
		"���� ��� ������ ESME_RSYSERR � ���� command_status, �� �� ������� SC ������������� �������� ������������ ������ (transaction rollback ��� ���������� ���������)");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidMsgId",
		"���� ��� ������ ESME_RINVMSGID � ���� command_status, �� ������������� message_id ����� �����������");
	__reg_tc__("replaceSm.resp.checkCmdStatusReplaceFiled",
		"���� ��� ������ ESME_RREPLACEFAIL � ���� command_status, �� ������������� ���������� ��������� ��������� � ��������� ���������");
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
	__reg_tc__("deliverySm.reports.deliveryReceipt.transmitter",
		"SC ���������� ������������� �������� �� �������� EXPIRED � ������ ������� validity_period, ���� sme-���������� ��������������� ��� transmitter");
	__reg_tc__("deliverySm.reports.deliveryReceipt.notBound",
		"SC ���������� ������������� �������� �� �������� EXPIRED � ������ ������� validity_period, ���� ��� ���������� � sme-�����������");
	__reg_tc__("deliverySm.reports.deliveryReceipt.failureDeliveryReceipt",
		"������������� �������� �� ������ �� ������������ � ������ �������� �������� ������������� ���������");
	__reg_tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt",
		"������������� �������� ��� ��������� ����� ���������� ������������ � ������ ������� validity_period, ���� ���� ��������� ������� �������� ������ ������ validity_period");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkAllowed",
		"�������� ������������� ��������� ������������� �������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� �� ��������� �������� ������������ pdu)");
	__reg_tc__("deliverySm.reports.deliveryReceipt.recvTimeChecks",
		"������������� �������� �������� � ������ ��������� �������� ������������ pdu");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkStatus",
		"���������� � ������� ������������ pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkText",
		"����� ��������� ������������� ���������� SC");
	//deliverySm.reports.intermediateNotification
	__reg_tc__("deliverySm.reports.intermediateNotification",
		"������������� �����������");
	__reg_tc__("deliverySm.reports.intermediateNotification.transmitter",
		"SC ���������� ������������� ����������� � ������ ������� schedule_delivery_time, ���� sme-���������� ��������������� ��� transmitter");
	__reg_tc__("deliverySm.reports.intermediateNotification.notBound",
		"SC ���������� ������������� ����������� � ������ ������� schedule_delivery_time, ���� ��� ���������� � sme-�����������");
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

void querySmTc()
{
	__reg_tc__("querySm", "���� ����� ��� query_sm");
	__reg_tc__("querySm.sync",
		"�������� ����������� query_sm pdu");
	__reg_tc__("querySm.async",
		"�������� ������������ query_sm pdu");
	__reg_tc__("querySm.receiver",
		"�������� query_sm �������� � sme ������������������ ��� receiver (SmppSession ������� exception)");
	__reg_tc__("querySm.transmitter",
		"�������� query_sm �������� � sme ������������������ ��� transmitter");
	__reg_tc__("querySm.transceiver",
		"�������� query_sm �������� � sme ������������������ ��� transceiver");
	//querySm.correct
	__reg_tc__("querySm.correct",
		"������ ������� ������������� sms");
	__reg_tc__("querySm.correct.enroute",
		"��������� ����������� � ��������� ENROUTE");
	__reg_tc__("querySm.correct.delivered",
		"��������� ����������� � ��������� DELIVERED");
	__reg_tc__("querySm.correct.expired",
		"��������� ����������� � ��������� EXPIRED");
	__reg_tc__("querySm.correct.undeliverable",
		"��������� ����������� � ��������� UNDELIVERABLE");
	__reg_tc__("querySm.correct.deleted",
		"��������� ����������� � ��������� DELETED");
	//querySm.incorrect
	__reg_tc__("querySm.incorrect",
		"������ ������� ��������������� sms");
	__reg_tc__("querySm.incorrect.messageId",
		"������������ message_id");
	__reg_tc__("querySm.incorrect.sourceAddr",
		"�������� message_id ����������, �� source_addr �� ���������");
	//querySm.resp
	__reg_tc__("querySm.resp", "��������� query_sm_resp pdu");
	__reg_tc__("querySm.resp.sync",
		"��������� query_sm_resp pdu ��� ���������� query_sm ��������");
	__reg_tc__("querySm.resp.async",
		"��������� query_sm_resp pdu ��� ����������� query_sm ��������");
	__reg_tc__("querySm.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("querySm.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("querySm.resp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("querySm.resp.checkFields",
		"���������� �������� ����� message_id, final_date, message_state, error_code");
	__reg_tc__("querySm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status ������� query_sm ������������� �� �������� ������ (��������� ����������, ����� ����������� ��������� � �.�.)");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� source_addr �� ��������� � ������� ����������� ��������� � ��");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidMsgId",
		"���� ��� ������ ESME_RINVMSGID � ���� command_status, �� ������������� message_id ����� �����������");
	__reg_tc__("querySm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void cancelSmTc()
{
	__reg_tc__("cancelSm", "���� ����� ��� cancel_sm");
	__reg_tc__("cancelSm.sync",
		"�������� ����������� cancel_sm pdu");
	__reg_tc__("cancelSm.async",
		"�������� ������������ cancel_sm pdu");
	__reg_tc__("cancelSm.receiver",
		"�������� cancel_sm �������� � sme ������������������ ��� receiver (SmppSession ������� exception)");
	__reg_tc__("cancelSm.transmitter",
		"�������� cancel_sm �������� � sme ������������������ ��� transmitter");
	__reg_tc__("cancelSm.transceiver",
		"�������� cancel_sm �������� � sme ������������������ ��� transceiver");
	//cancelSm.correct
	__reg_tc__("cancelSm.correct",
		"�������� cancel_sm �������� � ����������� ���������� �����");
	__reg_tc__("cancelSm.correct.messageIdWithoutDestAddr",
		"����� message_id ������������� ��������� � ENROUTE ���������, source_addr ���������, dest_addr � service_type �������");
	__reg_tc__("cancelSm.correct.messageIdWithDestAddr",
		"����� message_id ������������� ��������� � ENROUTE ���������, source_addr � dest_addr ���������, service_type �������");
	__reg_tc__("cancelSm.correct.destAddrWithoutServiceType",
		"������ ���������� source_addr � dest_addr � ������� message_id � service_type");
	__reg_tc__("cancelSm.correct.destAddrWithServiceType",
		"������ ���������� source_addr, dest_addr � service_type � ������� message_id");
	//cancelSm.incorrect
	__reg_tc__("cancelSm.incorrect",
		"�������� cancel_sm �������� � ������������� ���������� �����");
	__reg_tc__("cancelSm.incorrect.messageId",
		"�������������� message_id � ������� dest_addr � service_type");
	__reg_tc__("cancelSm.incorrect.sourceAddr",
		"����� message_id ������������� ��������� � ENROUTE ���������, dest_addr � service_type �������, �� source_addr �� ���������");
	__reg_tc__("cancelSm.incorrect.destAddr",
		"����� message_id ������������� ��������� � ENROUTE ���������, ����������� source_addr, ������� service_type � ������������� dest_addr");
	__reg_tc__("cancelSm.incorrect.sourceAddrOnly",
		"����� ������ source_addr, � message_id, dest_addr � service_type �������");
	__reg_tc__("cancelSm.incorrect.destAddrOnly",
		"����� ������ dest_addr, � message_id, source_addr � service_type �������");
	__reg_tc__("cancelSm.incorrect.messageIdOnly",
		"����� ������ message_id, � source_addr, dest_addr � service_type �������");
	__reg_tc__("cancelSm.incorrect.emptyFields",
		"����� ���������� source_addr �  ������ message_id, dest_addr � service_type");
	__reg_tc__("cancelSm.incorrect.serviceTypeWithoutDestAddr",
		"������ ���������� source_addr � service_type, �� dest_addr �������");
	__reg_tc__("cancelSm.incorrect.messageIdWithServiceType",
		"������ ������������ message_id � service_type ��� ������������� ��������� � ENROUTE ���������");
	__reg_tc__("cancelSm.incorrect.allFields",
		"������ ��� ���� message_id, source_addr, dest_addr � service_type ��� ������������� ��������� � ENROUTE ���������");
	__reg_tc__("cancelSm.incorrect.cancelFinal",
		"����� message_id ������������� ��������� � ��������� ���������, source_addr ���������, dest_addr � service_type �������");
	__reg_tc__("cancelSm.incorrect.noSms",
		"������ ���������� source_addr, dest_addr � service_type � ������� message_id, �� ��� ��������� � �� ��������������� �������� �������");
	//cancelSm.resp
	__reg_tc__("cancelSm.resp",
		"��������� cancel_sm_resp pdu");
	__reg_tc__("cancelSm.resp.sync",
		"��������� cancel_sm_resp pdu ��� ���������� cancel_sm ��������");
	__reg_tc__("cancelSm.resp.async",
		"��������� cancel_sm_resp pdu ��� ����������� cancel_sm ��������");
	__reg_tc__("cancelSm.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("cancelSm.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("cancelSm.resp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("cancelSm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status ������� cancel_sm ������������� �� �������� ������ (��������� ���������� � ��������� � ENROUTE ���������, ������ source_addr � dest_addr ������ ��������� � �.�.)");
	/*
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� source_addr ������������� �� ������������� source_addr ������������� ��������� � ��");
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidDestAddr",
		"���� ��� ������ ESME_RINVDSTADR � ���� command_status, �� dest_addr ������������� �� ������������� dest_addr ������������� ��������� � ��");
	*/
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidMsgId",
		"���� ��� ������ ESME_RINVMSGID � ���� command_status, �� ������������� message_id ����� �����������");
	__reg_tc__("cancelSm.resp.checkCmdStatusCancelFailed",
		"���� ��� ������ ESME_RCANCELFAIL � ���� command_status, �� ������������� ���� ������� ������������ (source_addr ��� dest_addr �� ��������� � ������������ ���������� � �.�.), ���� ��� ��������� ��������������� �������� (��� ���������, ��������� � ��������� ��������� � �.�.)");
	__reg_tc__("cancelSm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
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
	//protocolError.invalidBind
	__reg_tc__("protocolError.invalidBind",
		"�������� ������������ bind pdu (����� ����� ������������ ���������� � SC)");
	//protocolError.invalidBind.pduSize
	__reg_tc__("protocolError.invalidBind.pduSize",
		"�������� bind pdu ������������ �������");
	__reg_tc__("protocolError.invalidBind.pduSize.smallerSize1",
		"������ pdu ������ 16 ���� (������ ������)");
	__reg_tc__("protocolError.invalidBind.pduSize.smallerSize2",
		"������ pdu ������ �����������");
	__reg_tc__("protocolError.invalidBind.pduSize.greaterSize1",
		"������ pdu ������ �����������, �� ������ 64kb");
	__reg_tc__("protocolError.invalidBind.pduSize.greaterSize2",
		"������ pdu ������ 100kb");
	__reg_tc__("protocolError.invalidBind.pduSize.connectionClose",
		"��� ��������� bind pdu ������������� ������� SC ��������� ����������");
	//protocolError.invalidBind.cmdId
	__reg_tc__("protocolError.invalidBind.cmdId",
		"�������� pdu � command_id �������� �� bind_transmitter, bind_receiver � bind_transceiver");
	__reg_tc__("protocolError.invalidBind.cmdId.allowedCmdId",
		"���������� pdu � command_id ����������� ��� �������� �� ������� sme");
	__reg_tc__("protocolError.invalidBind.cmdId.unbindBeforeBind",
		"�������� unbind ��������");
	__reg_tc__("protocolError.invalidBind.cmdId.notAllowedCmdId",
		"���������� pdu � command_id ����������� ��� �������� ������������� �� ������� SC");
	__reg_tc__("protocolError.invalidBind.cmdId.nonExistentCmdId",
		"������������ pdu � command_id ����������������� ������������");
	__reg_tc__("protocolError.invalidBind.cmdId.generickNack",
		"��� ��������� pdu � command_id �������� �� bind SC ���������� generic_nack � command_status = ESME_RINVBNDSTS");
	//protocolError.invalidPdu
	__reg_tc__("protocolError.invalidPdu",
		"�������� ������������ pdu ����� ������������ ���������� � ��������� bind");
	//protocolError.invalidPdu.pduSize
	__reg_tc__("protocolError.invalidPdu.pduSize",
		"�������� pdu ������������� �������");
	__reg_tc__("protocolError.invalidPdu.pduSize.smallerSize1",
		"������ pdu ������ 16 ���� (������ ������)");
	__reg_tc__("protocolError.invalidPdu.pduSize.smallerSize2",
		"������ pdu ������ �����������");
	__reg_tc__("protocolError.invalidPdu.pduSize.greaterSize1",
		"������ pdu ������ �����������, �� ������ 64kb");
	__reg_tc__("protocolError.invalidPdu.pduSize.greaterSize2",
		"������ pdu ������ 100kb");
	__reg_tc__("protocolError.invalidPdu.pduSize.connectionClose",
		"��� ��������� pdu ������������� ������� SC ��������� ����������");
	//protocolError.invalidPdu.cmdId
	__reg_tc__("protocolError.invalidPdu.cmdId",
		"�������� pdu � ������������ command_id");
	__reg_tc__("protocolError.invalidPdu.cmdId.notAllowedCmdId",
		"���������� pdu � command_id ����������� ��� �������� ������������� �� ������� SC");
	__reg_tc__("protocolError.invalidPdu.cmdId.nonExistentCmdId",
		"������������ pdu � command_id ����������������� ������������");
	__reg_tc__("protocolError.invalidPdu.cmdId.generickNack",
		"��� ��������� pdu � ������������ command_id SC ���������� generic_nack � command_status = ESME_RINVCMDID");
	//protocolError.submitAfterUnbind
	__reg_tc__("protocolError.submitAfterUnbind",
		"�������� pdu ����� unbind (������� ���������� �������� ��������)");
	//protocolError.submitAfterUnbind.pduSize
	__reg_tc__("protocolError.submitAfterUnbind.pduSize",
		"�������� pdu ������������� �������");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.smallerSize1",
		"������ pdu ������ 16 ���� (������ ������)");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.smallerSize2",
		"������ pdu ������ �����������");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.greaterSize1",
		"������ pdu ������ �����������, �� ������ 64kb");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.greaterSize2",
		"������ pdu ������ 100kb");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.connectionClose",
		"��� ��������� pdu ������������� ������� SC ��������� ����������");
	//protocolError.submitAfterUnbind.cmdId
	__reg_tc__("protocolError.submitAfterUnbind.cmdId",
		"�������� pdu � command_id �������� �� bind_transmitter, bind_receiver � bind_transceiver");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.allowedCmdId",
		"���������� pdu � command_id ����������� ��� �������� �� ������� sme");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.duplicateUnbind",
		"��������� unbind �������");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.notAllowedCmdId",
		"���������� pdu � command_id ����������� ��� �������� ������������� �� ������� SC");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.nonExistentCmdId",
		"������������ pdu � command_id ����������������� ������������");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.generickNack",
		"��� ��������� pdu � command_id �������� �� bind SC ���������� generic_nack � command_status = ESME_RINVBNDSTS");
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
	__reg_tc__("notImplemented.submitMulti.resp", "��������� submit_multi_resp pdu");
	__reg_tc__("notImplemented.dataSm", "�������� data_sm pdu");
	__reg_tc__("notImplemented.dataSm.delivery", "��������� data_sm pdu");
	__reg_tc__("notImplemented.dataSm.resp", "��������� data_sm_resp pdu");
	__reg_tc__("notImplemented.alertNotification", "��������� alert_notification pdu");
}

void allProtocolTc()
{
	bindTc();
	unbindTc();
	enquireLinkTc();
	submitSmTc();
	replaceSmTc();
	deliverySmTc();
	querySmTc();
	cancelSmTc();


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

