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
	__reg_tc__("bind.correct.afterUnbind",
		"�������� bind �������� ����� unbind ��� ���� �� ������ tcp/ip ����������");
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
	__reg_tc__("unbind.secondUnbind",
		"�������� ���������� unbind ��������");
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
	__reg_tc__("submitSm.forceDc",
		"�������� submit_sm �������� � sme � ������������� ��������� forceDC");
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
		"���������� �������� ����� ���� short_message");
	__reg_tc__("submitSm.correct.messagePayloadLengthMarginal",
		"���������� �������� ����� ���� message_payload");
	__reg_tc__("submitSm.correct.ussdRequest",
		"�������� ����������� ussd �������");
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
	__reg_tc__("submitSm.incorrect.dataCodingNormalSme",
		"������������ �������� dataCoding ��� ���������� sme");
	__reg_tc__("submitSm.incorrect.dataCodingForceDcSme",
		"������������ �������� dataCoding ��� ������� sme");
	__reg_tc__("submitSm.incorrect.bothMessageFields",
		"������ ��� ���� short_message � message_payload");
	__reg_tc__("submitSm.incorrect.udhiLength",
		"����� udh ��� ������������ ����� udhi ������ �����������");
	__reg_tc__("submitSm.incorrect.serviceTypeLength",
		"����� ���� service_type ������ ����������");
	__reg_tc__("submitSm.incorrect.sourceAddrLength",
		"����� ���� source_addr ������ ����������");
	__reg_tc__("submitSm.incorrect.destAddrLength",
		"����� ���� dest_addr ������ ����������");
	__reg_tc__("submitSm.incorrect.validTimeLength",
		"����� ���� validity_period ������ ����������");
	__reg_tc__("submitSm.incorrect.waitTimeLength",
		"����� ���� schedule_delivery_time ������ ����������");
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
		"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� �������� ���� schedule_delivery_time ��� ��������� def ������������� ������ �����������");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidValidTime",
		"���� ��� ������ ESME_RINVEXPIRY � ���� command_status, �� �������� ���� validity_period ������������� ������ �����������");
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
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidMsgLen",
		"���� ��� ������ ESME_RINVMSGLEN � ���� command_status, �� ������������� ����� ��������� ������������ (��������, ������� udhi � ������� MT ���������)");
	__reg_tc__("submitSm.resp.checkCmdStatusSubmitFailed",
		"���� ��� ������ ESME_RSUBMITFAIL � ���� command_status, �� pdu ������������� ������������ (������������ ��� ���� short_message � message_payload � �.�.)");
	__reg_tc__("submitSm.resp.checkCmdStatusNoRoute",
		"���� ��� ������ NOROUTE � ���� command_status, �� �������� ������������� �� ����������");
	__reg_tc__("submitSm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void dataSmTc()
{
	__reg_tc__("dataSm", "���� ����� ��� data_sm");
	__reg_tc__("dataSm.sync",
		"�������� ����������� data_sm ��������");
	__reg_tc__("dataSm.async",
		"�������� ������������ data_sm ��������");
	__reg_tc__("dataSm.receiver",
		"�������� data_sm �������� � sme ������������������ ��� receiver (SmppSession ������� exception)");
	__reg_tc__("dataSm.transmitter",
		"�������� data_sm �������� � sme ������������������ ��� transmitter");
	__reg_tc__("dataSm.transceiver",
		"�������� data_sm �������� � sme ������������������ ��� transceiver");
	__reg_tc__("dataSm.forceDc",
		"�������� data_sm �������� � sme � ������������� ��������� forceDC");
	//dataSm.correct
	__reg_tc__("dataSm.correct",
		"�������� data_sm pdu � ����������� ���������� �����");
	__reg_tc__("dataSm.correct.serviceTypeMarginal",
		"���������� �������� ����� ���� service_type");
	__reg_tc__("dataSm.correct.validTimeExceeded",
		"����� qos_time_to_live ������ ����������� ����� ���������� �������������� ��� SC");
	__reg_tc__("dataSm.correct.messagePayloadLengthMarginal",
		"���������� �������� ����� ���� message_payload");
	__reg_tc__("dataSm.correct.ussdRequest",
		"�������� ����������� ussd �������");
	__reg_tc__("dataSm.correct.notReplace",
		"������������� ��������� (��������� source_addr, dest_addr � service_type �� ���������� ��������� ������������ � ������� ��������)");
	//dataSm.incorrect
	__reg_tc__("dataSm.incorrect",
		"�������� data_sm pdu � ������������� ���������� �����");
	__reg_tc__("dataSm.incorrect.sourceAddr",
		"������������ ����� �����������");
	__reg_tc__("dataSm.incorrect.destAddr",
		"������������ ����� ����������");
	__reg_tc__("dataSm.incorrect.dataCodingNormalSme",
		"������������ �������� dataCoding ��� ���������� sme");
	__reg_tc__("dataSm.incorrect.dataCodingForceDcSme",
		"������������ �������� dataCoding ��� ������� sme");
	__reg_tc__("dataSm.incorrect.udhiLength",
		"����� udh ��� ������������ ����� udhi ������ �����������");
	__reg_tc__("dataSm.incorrect.serviceTypeLength",
		"����� ���� service_type ������ ����������");
	__reg_tc__("dataSm.incorrect.sourceAddrLength",
		"����� ���� source_addr ������ ����������");
	__reg_tc__("dataSm.incorrect.destAddrLength",
		"����� ���� dest_addr ������ ����������");
	__reg_tc__("dataSm.incorrect.validTimeLength",
		"����� ���� validity_period ������ ����������");
	__reg_tc__("dataSm.incorrect.waitTimeLength",
		"����� ���� schedule_delivery_time ������ ����������");
	__reg_tc__("dataSm.incorrect.transactionRollback",
		"����������� sms ����������� transaction rollback �� ������� ������� ��");
	//dataSm.assert
	__reg_tc__("dataSm.assert",
		"���������� � �������� data_sm pdu � ������������� ���������� �����");
	__reg_tc__("dataSm.assert.serviceTypeInvalid",
		"������������ �������� ����� ���� service_type");
	__reg_tc__("dataSm.assert.destAddrLengthInvalid",
		"������������ �������� ����� ���� ������ ����������");
	//dataSm.resp
	__reg_tc__("dataSm.resp",
		"��������� data_sm_resp pdu");
	__reg_tc__("dataSm.resp.sync",
		"��������� data_sm_resp pdu ��� ���������� data_sm ��������");
	__reg_tc__("dataSm.resp.async",
		"��������� data_sm_resp pdu ��� ����������� data_sm ��������");
	__reg_tc__("dataSm.resp.checkDuplicates",
		"�� ������ ������� �������� ������������ �������");
	__reg_tc__("dataSm.resp.checkTime",
		"���������� ����� ��������� ��������");
	__reg_tc__("dataSm.resp.checkHeader",
		"���������� �������� ����� ������ �������� (command_length, command_id, sequence_number)");
	__reg_tc__("dataSm.resp.checkOptional",
		"���������� �������� ������������ ����� �������� (delivery_failure_reason, network_error_code, additional_status_info_text � dpf_result)");
	__reg_tc__("dataSm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status, ����������� ��� ������� ��� ���������� �������� ��������� (���� �������� ������ ���������, ���������� ������� � �.�.)");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidDestAddr",
		"���� ��� ������ ESME_RINVDSTADR � ���� command_status, �� �������� ������������� �� ����������");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidWaitTime",
		"���� ��� ������ ESME_RINVSCHED � ���� command_status, �� �������� ��������� def ������������� ������ �����������");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidDataCoding",
		"���� ��� ������ ESME_RINVDCS � ���� command_status, �� ��������� ��������� ������������� ������ �����������");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidServiceType",
		"���� ��� ������ ESME_RINVSERTYP � ���� command_status, �� ����� ���� service_type ������������� ������ �����������");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� ����� ����������� ������������� �� ������������� address range ��� ������ sme � ������������ sme.xml SC");
	__reg_tc__("dataSm.resp.checkCmdStatusSystemError",
		"���� ��� ������ ESME_RSYSERR � ���� command_status, �� �� ������� SC ������������� �������� ������������ ������ (transaction rollback ��� ���������� ���������)");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidMsgLen",
		"���� ��� ������ ESME_RINVMSGLEN � ���� command_status, �� ������������� ����� ��������� ������������ (��������, ������� udhi � ������� MT ���������)");
	__reg_tc__("dataSm.resp.checkCmdStatusSubmitFailed",
		"���� ��� ������ ESME_RSUBMITFAIL � ���� command_status, �� pdu ������������� ������������ (������������ ��� ���� short_message � message_payload � �.�.)");
	__reg_tc__("dataSm.resp.checkCmdStatusNoRoute",
		"���� ��� ������ NOROUTE � ���� command_status, �� �������� ������������� �� ����������");
	__reg_tc__("dataSm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
}

void directiveTc()
{
	__reg_tc__("directive.submitSm",
		"�������� submit_sm pdu � ����������� (� short_message ��� message_payload)");
	__reg_tc__("directive.dataSm",
		"�������� data_sm pdu � ����������� (� message_payload)");
	__reg_tc__("directive.mixedCase",
		"����� ��������� � ��������� ��������");
	__reg_tc__("directive.correct",
		"�������� sms � ����������� �����������");
	__reg_tc__("directive.correct.ack",
		"���������� ack ���������");
	__reg_tc__("directive.correct.noack",
		"���������� noack ���������");
	__reg_tc__("directive.correct.def",
		"���������� def ���������");
	__reg_tc__("directive.correct.template",
		"���������� template ��������� � ��������� ����������� ���������� (0..2), ������� � ��������� ������� � ���������� � �.�.");
	__reg_tc__("directive.incorrect",
		"�������� sms � ������������� �����������");
	__reg_tc__("directive.incorrect.invalidDir",
		"�������������� ���������");
	__reg_tc__("directive.incorrect.invalidDefDir",
		"����������� �������� ��������� ��� def ���������");
	__reg_tc__("directive.incorrect.invalidTemplateDir",
		"����������� �������� ��������� ��� template ���������");
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
	__reg_tc__("replaceSm.map",
		"�������� replace_sm �������� ��� sms, ������� ����� ������������ �� map proxy");
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
	__reg_tc__("replaceSm.incorrect.udhiLength",
		"����� udh ��� ������������ ����� udhi (� ���������� ���������) ������ �����������");
	__reg_tc__("replaceSm.incorrect.messageIdLength",
		"����� ���� message_id ������ ����������");
	__reg_tc__("replaceSm.incorrect.sourceAddrLength",
		"����� ���� source_addr ������ ����������");
	__reg_tc__("replaceSm.incorrect.validTimeLength",
		"����� ���� validity_period ������ ����������");
	__reg_tc__("replaceSm.incorrect.waitTimeLength",
		"����� ���� schedule_delivery_time ������ ����������");
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
	__reg_tc__("replaceSm.resp.checkCmdStatusReplaceFailed",
		"���� ��� ������ ESME_RREPLACEFAIL � ���� command_status, �� ������������� ���������� ��������� ��������� � ��������� ���������");
	__reg_tc__("replaceSm.resp.checkCmdStatusOther",
		"������ ���� ������ ������������� ������������");
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
	__reg_tc__("querySm.incorrect.messageIdLength",
		"����� ���� message_id ������ ����������");
	__reg_tc__("querySm.incorrect.sourceAddrLength",
		"����� ���� source_addr ������ ����������");
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
	__reg_tc__("querySm.resp.checkFields.enroute",
		"��������� ����������� � ��������� ENROUTE");
	__reg_tc__("querySm.resp.checkFields.delivered",
		"��������� ����������� � ��������� DELIVERED");
	__reg_tc__("querySm.resp.checkFields.expired",
		"��������� ����������� � ��������� EXPIRED");
	__reg_tc__("querySm.resp.checkFields.undeliverable",
		"��������� ����������� � ��������� UNDELIVERABLE");
	__reg_tc__("querySm.resp.checkFields.deleted",
		"��������� ����������� � ��������� DELETED");
	__reg_tc__("querySm.resp.checkCmdStatusOk",
		"��� ���������� ���� ������ � ���� command_status ������� query_sm ������������� �� �������� ������ (��������� ����������, ����� ����������� ��������� � �.�.)");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidSourceAddr",
		"���� ��� ������ ESME_RINVSRCADR � ���� command_status, �� source_addr �� ��������� � ������� ����������� ��������� � ��");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidBindStatus",
		"���� ��� ������ ESME_RINVBNDSTS � ���� command_status, �� ������������� sme ����������������� ��� receiver");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidMsgId",
		"���� ��� ������ ESME_RINVMSGID � ���� command_status, �� ������������� message_id ����� �����������");
	__reg_tc__("querySm.resp.checkCmdStatusQueryFail",
		"���� ��� ������ ESME_RQUERYFAIL � ���� command_status, �� ������������� ��������� query_sm �� ����� ���� ��������� ���������");
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
	__reg_tc__("cancelSm.incorrect.messageIdLength",
		"����� ���� message_id ������ ����������");
	__reg_tc__("cancelSm.incorrect.serviceTypeLength",
		"����� ���� service_type ������ ����������");
	__reg_tc__("cancelSm.incorrect.sourceAddrLength",
		"����� ���� source_addr ������ ����������");
	__reg_tc__("cancelSm.incorrect.destAddrLength",
		"����� ���� dest_addr ������ ����������");
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

void smsTc()
{
	__reg_tc__("sms", "��������� sms �� SC");
	__reg_tc__("sms.deliverySm", "��������� deliver_sm");
	__reg_tc__("sms.deliverySm.checkFields",
		"����� �������� ����� ������ � ��������� ������ ����� deliver_sm");
	__reg_tc__("sms.dataSm", "��������� data_sm");
	__reg_tc__("sms.dataSm.checkFields",
		"����� �������� ����� ������ � ��������� ������ ����� data_sm");
}

void normalSmsTc()
{
	//sms.normalSms
	__reg_tc__("sms.normalSms",
		"��������� ������� sms (deliver_sm, data_sm) �� ���������� ������������");
	__reg_tc__("sms.normalSms.checkAllowed",
		"�������� ������������� ��������� sms");
	__reg_tc__("sms.normalSms.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ����������)");
	__reg_tc__("sms.normalSms.checkMandatoryFields",
		"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm, data_sm) pdu");
	__reg_tc__("sms.normalSms.checkOptionalFields",
		"��������� ������������ ����� ������������ (submit_sm, data_sm, replace_sm) � ���������� (deliver_sm, data_sm) pdu");
	__reg_tc__("sms.normalSms.scheduleChecks",
		"���������� ������ ��������� ��������� �������� (���������� �����, ��� ��������� ����� ���������� ����������, ���������� ������)");
	__reg_tc__("sms.normalSms.ussdServiceOp",
		"���� ����������� ������������ ���� ussd_service_op, �� SC ��������� ������������ ������� �������� ���������");
	//sms.normalSms.notMap
	__reg_tc__("sms.normalSms.notMap",
		"Sms ����������� ������� �������� sme, � �� ��������� ��������");
	__reg_tc__("sms.normalSms.notMap.checkDeliverSm",
		"���� sms ������������ ��� deliver_sm, �� ������������ sms ���� ���� ���������� ��� submit_sm, ���� ���� �������� replace_sm ��� submit_sm � ������������� replace_if_present");
	__reg_tc__("sms.normalSms.notMap.checkDataSm",
		"���� sms ������������ ��� data_sm, �� ������������ sms ���� ���������� ��� data_sm � �� ���� �������� replace_sm � submit_sm");
	__reg_tc__("sms.normalSms.notMap.checkDataCoding",
		"��������� ������ ����������� sms ������������� ���������� ������� ���������� �� ������ ������ ��������");
	__reg_tc__("sms.normalSms.notMap.checkEqualDataCoding",
		"���� ��������� ���������� � ���������, ������� �������� ����������, ���� short_message (��� deliver_sm) �/��� message_payload ���������� ��� ���������");
	__reg_tc__("sms.normalSms.notMap.checkDiffDataCoding",
		"���� ��������� ���������� � ucs2, � � ������� ���������� ��������� default, �� �������� ����� short_message (��� deliver_sm) �/��� message_payload ��������� ����������������� � ������ udh");
	//sms.normalSms.map
	__reg_tc__("sms.normalSms.map",
		"Sms ����������� ������� �������� ��������� ��������");
	__reg_tc__("sms.normalSms.map.checkDataCoding",
		"��������� ������ ����������� sms ������������� ���������� ������� ���������� �� ������ �������� ���������");
	//sms.normalSms.map.shortSms
	__reg_tc__("sms.normalSms.map.shortSms",
		"�������� ��������� (<=140 ����) ������������ ����� sms");
	__reg_tc__("sms.normalSms.map.shortSms.checkDeliverSm",
		"���� sms ������������ ��� deliver_sm, �� ������������ sms ���� ���� ���������� ��� submit_sm, ���� ���� �������� replace_sm ��� submit_sm � ������������� replace_if_present");
	__reg_tc__("sms.normalSms.map.shortSms.checkDataSm",
		"���� sms ������������ ��� data_sm, �� ������������ sms ���� ���������� ��� data_sm � �� ���� �������� replace_sm � submit_sm");
	__reg_tc__("sms.normalSms.map.shortSms.checkEqualDataCoding",
		"���� ��������� ���������� � ���������, ������� �������� ����������, ���� short_message (��� deliver_sm) �/��� message_payload ���������� ��� ���������");
	__reg_tc__("sms.normalSms.map.shortSms.checkDiffDataCoding",
		"���� ��������� ���������� � ucs2, � � ������� ���������� ��������� default, �� �������� ����� short_message (��� deliver_sm) �/��� message_payload ��������� ����������������� � ������ udh");
	//sms.normalSms.map.longSms
	__reg_tc__("sms.normalSms.map.longSms",
		"������� ��������� (>140 ����) ��� udhi ���������� �� ��������");
	__reg_tc__("sms.normalSms.map.longSms.checkDeliverSm",
		"�������� ������������ ��� deliver_sm ���������� �� ���� ���� ������������ sms ���������� ��� submit_sm ��� ��� data_sm");
	__reg_tc__("sms.normalSms.map.longSms.checkEqualDataCoding",
		"���� ��������� ���������� � ���������, ������� �������� ����������, �������� ������������� ������ ������������ � ���� short_message");
	__reg_tc__("sms.normalSms.map.longSms.checkDiffDataCoding",
		"���� ��������� ���������� � ucs2, � � ������� ���������� ��������� default, �� �������� �������������������� ������ ������������ � ���� short_message");
}

void reportsTc()
{
	//sms.reports
	__reg_tc__("sms.reports",
		"������ � �������� (������������� �������� � ������������� �����������)");
	__reg_tc__("sms.reports.checkDeliverSm",
		"������ � �������� ������ ������������ ��� deliver_sm ���������� �� ����, ���� ���������� ������������ sms ��� submit_sm ��� ��� data_sm");
	__reg_tc__("sms.reports.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ SC)");
	__reg_tc__("sms.reports.checkFields",
		"����� �������� ������������ �����");
	//sms.reports.priorityCheck
	__reg_tc__("sms.reports.priorityCheck",
		"���������� �������� ������������� �������� ������ � ��������");
	__reg_tc__("sms.reports.priorityCheck.ussdServiceOp",
		"���� ����������� ������������ ���� ussd_service_op, �� SC �� ���������� ������ � �������� (���������� �� ������� ��������� #ack#)");
	__reg_tc__("sms.reports.priorityCheck.directiveAck",
		"���� ������ ��������� #ack#, �� SC ���������� ������ � �������� (���������� �� ������� �� �������� ����� suppressDeliveryReports)");
	__reg_tc__("sms.reports.priorityCheck.directiveNoAck",
		"���� ������ ��������� #noack#, �� SC �� ���������� ������ � �������� (���������� �� �������� ���� registered_delivery � pdu)");
	__reg_tc__("sms.reports.priorityCheck.suppressDeliveryReports",
		"���� �� �������� ���������� ���� suppressDeliveryReports, �� SC �� ���������� ������ � �������� (���������� �� �������� ���� registered_delivery � pdu)");
	__reg_tc__("sms.reports.priorityCheck.profileReportFull",
		"���� � ������� ����������� �� ������ �������� sms ���������� ����� ������� REPORT_FULL, �� SC ���������� ������ � �������� (���������� �� �������� ���� registered_delivery � pdu)");
	//sms.reports.deliveryReceipt
	__reg_tc__("sms.reports.deliveryReceipt",
		"������������� ��������");
	__reg_tc__("sms.reports.deliveryReceipt.transmitter",
		"SC ���������� ������������� �������� �� �������� EXPIRED � ������ ������� validity_period, ���� sme-���������� ��������������� ��� transmitter");
	__reg_tc__("sms.reports.deliveryReceipt.notBound",
		"SC ���������� ������������� �������� �� �������� EXPIRED � ������ ������� validity_period, ���� ��� ���������� � sme-�����������");
	__reg_tc__("sms.reports.deliveryReceipt.failureDeliveryReceipt",
		"������������� �������� �� ������ �� ������������ � ������ �������� �������� ������������� ���������");
	__reg_tc__("sms.reports.deliveryReceipt.expiredDeliveryReceipt",
		"������������� �������� ��� ��������� ����� ���������� ������������ � ������ ������� validity_period, ���� ���� ��������� ������� �������� ������ ������ validity_period");
	__reg_tc__("sms.reports.deliveryReceipt.checkAllowed",
		"�������� ������������� ��������� ������������� �������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� �� ��������� �������� ������������ pdu)");
	__reg_tc__("sms.reports.deliveryReceipt.recvTimeChecks",
		"������������� �������� �������� � ������ ��������� �������� ������������ pdu");
	__reg_tc__("sms.reports.deliveryReceipt.checkStatus",
		"���������� � ������� ������������ pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("sms.reports.deliveryReceipt.checkText",
		"����� ��������� ������������� ���������� SC");
	//sms.reports.intermediateNotification
	__reg_tc__("sms.reports.intermediateNotification",
		"������������� �����������");
	__reg_tc__("sms.reports.intermediateNotification.transmitter",
		"SC ���������� ������������� ����������� � ������ ������� schedule_delivery_time, ���� sme-���������� ��������������� ��� transmitter");
	__reg_tc__("sms.reports.intermediateNotification.notBound",
		"SC ���������� ������������� ����������� � ������ ������� schedule_delivery_time, ���� ��� ���������� � sme-�����������");
	__reg_tc__("sms.reports.intermediateNotification.checkAllowed",
		"�������� ������������� ��������� ������������� ����������� (� ����������� �� �������� ������� � ���� pdu registered_delivery, ������������ ��� ����� ������ ������������� ������� ��������)");
	__reg_tc__("sms.reports.intermediateNotification.noRescheduling",
		"������������� ����������� �� ������������ � ������, ���� sms �� ���� ������������");
	__reg_tc__("sms.reports.intermediateNotification.recvTimeChecks",
		"����� �������� ������������� ����������� ������������ ������� ������ ������������� ������� �������� ������������ pdu");
	__reg_tc__("sms.reports.intermediateNotification.checkStatus",
		"���������� � ������� pdu (���������, ��� ������) �������� ����������");
	__reg_tc__("sms.reports.intermediateNotification.checkText",
		"����� ��������� ������������� ���������� SC");
	//sms.reports.smsCancelledNotification
	__reg_tc__("sms.reports.smsCancelledNotification",
		"����������� �� ��������� sms");
}

void smeAckTc()
{
	//sms.smeAck
	__reg_tc__("sms.smeAck",
		"�������� ��������� �� ���������� � ������� sme (profiler, abonent info, db sme � �.�.)");
	__reg_tc__("sms.smeAck.checkDeliverSm",
		"�������� ��������� ������ ������������ ��� deliver_sm ���������� �� ����, ���� ���������� ������������ sms ��� submit_sm ��� ��� data_sm");
	__reg_tc__("sms.smeAck.checkAllowed",
		"�������� ������������� ��������� ���������");
	__reg_tc__("sms.smeAck.checkRoute",
		"�������� ������������ �������� (����������� sme �� ������ ����������� � ������ ���������� sme SC)");
	__reg_tc__("sms.smeAck.checkFields",
		"����� �������� ������������ �����");
	__reg_tc__("sms.smeAck.recvTimeChecks",
		"���������� ����� ��������� ���������");
}

void smsRespTc()
{
	__reg_tc__("smsResp", "�������� ��������� �� SC");
	//smsResp.deliverySm
	__reg_tc__("smsResp.deliverySm", "�������� deliver_sm_resp pdu");
	__reg_tc__("smsResp.deliverySm.sync",
		"�������� ����������� deliver_sm_resp pdu");
	__reg_tc__("smsResp.deliverySm.async",
		"�������� ������������ deliver_sm_resp pdu");
	__reg_tc__("smsResp.deliverySm.delay",
		"�������� deliver_sm_resp pdu � ��������� ������� sme timeout");
	__reg_tc__("smsResp.deliverySm.sendOk",
		"��������� ������� �������������� ��������� ����������� deliver_sm");
	//smsResp.deliverySm.sendRetry
	__reg_tc__("smsResp.deliverySm.sendRetry",
		"��������� ������� ��� ������������ ��������� �������� deliver_sm");
	__reg_tc__("smsResp.deliverySm.sendRetry.notSend",
		"�� ��������� �������, �������� ����������� deliver_sm");
	__reg_tc__("smsResp.deliverySm.sendRetry.tempAppError",
		"��������� ������ ESME_RX_T_APPN (��������� ������ �� ������� sme, ������ �� ��������� ��������)");
	__reg_tc__("smsResp.deliverySm.sendRetry.msgQueueFull",
		"��������� ������ ESME_RMSGQFUL (������������ ������� ������� sme)");
	__reg_tc__("smsResp.deliverySm.sendRetry.invalidSequenceNumber",
		"��������� ������� � ������������ sequence_number");
	__reg_tc__("smsResp.deliverySm.sendRetry.sendAfterSmeTimeout",
		"��������� ������� ����� sme timeout");
	//smsResp.deliverySm.sendError
	__reg_tc__("smsResp.deliverySm.sendError",
		"��������� ������� ��� ����������� ��������� �������� deliver_sm");
	__reg_tc__("smsResp.deliverySm.sendError.standardError",
		"������� �� ����������� ����� ������ (�������� 0x1-0x10f)");
	__reg_tc__("smsResp.deliverySm.sendError.reservedError",
		"������� � ����������������� ����� ������ (��������� 0x110-0x3ff � 0x400-0x4ff)");
	__reg_tc__("smsResp.deliverySm.sendError.outRangeError",
		"������� � ����� ������ ��� ��������� ������������� ������������� SMPP (>0x500)");
	__reg_tc__("smsResp.deliverySm.sendError.permanentAppError",
		"������� � ����� ������ ESME_RX_P_APPN (������������ ������ �� ������� sme, ����� �� ���� ����������� ���������)");
	//smsResp.dataSm
	__reg_tc__("smsResp.dataSm", "�������� data_sm_resp pdu");
	__reg_tc__("smsResp.dataSm.sync",
		"�������� ����������� data_sm_resp pdu");
	__reg_tc__("smsResp.dataSm.async",
		"�������� ������������ data_sm_resp pdu");
	__reg_tc__("smsResp.dataSm.delay",
		"�������� data_sm_resp pdu � ��������� ������� sme timeout");
	__reg_tc__("smsResp.dataSm.sendOk",
		"��������� ������� �������������� ��������� ����������� data_sm");
	//smsResp.dataSm.sendRetry
	__reg_tc__("smsResp.dataSm.sendRetry",
		"��������� ������� ��� ������������ ��������� �������� data_sm");
	__reg_tc__("smsResp.dataSm.sendRetry.notSend",
		"�� ��������� �������, �������� ����������� data_sm");
	__reg_tc__("smsResp.dataSm.sendRetry.tempAppError",
		"��������� ������ ESME_RX_T_APPN (��������� ������ �� ������� sme, ������ �� ��������� ��������)");
	__reg_tc__("smsResp.dataSm.sendRetry.msgQueueFull",
		"��������� ������ ESME_RMSGQFUL (������������ ������� ������� sme)");
	__reg_tc__("smsResp.dataSm.sendRetry.invalidSequenceNumber",
		"��������� ������� � ������������ sequence_number");
	__reg_tc__("smsResp.dataSm.sendRetry.sendAfterSmeTimeout",
		"��������� ������� ����� sme timeout");
	//smsResp.dataSm.sendError
	__reg_tc__("smsResp.dataSm.sendError",
		"��������� ������� ��� ����������� ��������� �������� data_sm");
	__reg_tc__("smsResp.dataSm.sendError.standardError",
		"������� �� ����������� ����� ������ (�������� 0x1-0x10f)");
	__reg_tc__("smsResp.dataSm.sendError.reservedError",
		"������� � ����������������� ����� ������ (��������� 0x110-0x3ff � 0x400-0x4ff)");
	__reg_tc__("smsResp.dataSm.sendError.outRangeError",
		"������� � ����� ������ ��� ��������� ������������� ������������� SMPP (>0x500)");
	__reg_tc__("smsResp.dataSm.sendError.permanentAppError",
		"������� � ����� ������ ESME_RX_P_APPN (������������ ������ �� ������� sme, ����� �� ���� ����������� ���������)");
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
	//protocolError.smeInactivity
	__reg_tc__("protocolError.smeInactivity",
		"���� sme ���������� ����� �� ��������� ����������, SC �������� ���������� �� sme enquire_link ��������, � ����� ��������� ����������");
	__reg_tc__("protocolError.smeInactivity.checkEnquireLinkTime",
		"������ enquire_link SC ���������� ����� ������ ������� inactivityTime ���������� � ������������, ��� ����������� ����� 1 �������");
	__reg_tc__("protocolError.smeInactivity.checkEnquireLinkFileds",
		"������� enquire_link ������������ SC �������� ����������");
	__reg_tc__("protocolError.smeInactivity.checkConnectionClose",
		"���� � ������� ������� enquire_link � ������� ������� inactivityTimeout ����������� � ������������ sme ������ �� ����������, �� SC ��������� ����������");
	//other
	__reg_tc__("protocolError.equalSeqNum",
		"�������� ���������� submit_sm � ���������� sequence_number �� ������ �� ������ ��������� pdu");
	__reg_tc__("protocolError.nullPdu",
		"�������� pdu � ������� (NULL) ������");
}

void lockTc()
{
	__reg_tc__("lockedSm", "��������� �������� � ����������� sms");
	__reg_tc__("lockedSm.deliveringState",
		"���� sms ��������� � DELIVERING ���������, �� ���������� cancel_sm, replace_sm, query_sm � submit_sm � ������������� replace_if_present ������������� �� ������ sms �� DELIVERING ��������� ��� ������� ��������� �� ��������");
	__reg_tc__("lockedSm.segmentedMap",
		"���� sms ������������ �� map proxy ������������ � ��������� �� ��������� � ���� �� ���� ������� ��� ���������, �� ���������� cancel_sm, replace_sm � submit_sm � ������������� replace_if_present ���������� �����������");
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

void notImplementedTc()
{
	__reg_tc__("notImplemented", "�� ������������������ ���� �����");
	__reg_tc__("notImplemented.submitMulti", "�������� submit_multi pdu");
	__reg_tc__("notImplemented.submitMulti.resp", "��������� submit_multi_resp pdu");
	__reg_tc__("notImplemented.alertNotification", "��������� alert_notification pdu");
}

void allProtocolTc()
{
	bindTc();
	unbindTc();
	enquireLinkTc();
	submitSmTc();
	dataSmTc();
	directiveTc();
	replaceSmTc();
	querySmTc();
	cancelSmTc();

	smsTc();
	normalSmsTc();
	reportsTc();
	smeAckTc();
	smsRespTc();

	checkMissingPduTc();
	protocolError();

	lockTc();
	sendInvalidPduTc();
	processGenericNackTc();
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

