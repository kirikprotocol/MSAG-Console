#include "SmppProtocolTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::conf::TestConfig;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::test::sms; //constants
using namespace smsc::test::smpp;
using namespace smsc::test::core; //constants
using namespace smsc::test::util;

SmppProtocolTestCases::SmppProtocolTestCases(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList) {}

Category& SmppProtocolTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppProtocolTestCases");
	return log;
}

void SmppProtocolTestCases::submitSmAssert(int num)
{
	TCSelector s(num, 4);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm pdu;
			switch (s.value())
			{
				case 1: //serviceType ������ ������������ �����
					{
						__tc__("submitSm.assert.serviceTypeInvalid");
						char serviceType[MAX_SERVICE_TYPE_LENGTH + 10];
						rand_char(MAX_SERVICE_TYPE_LENGTH + 1, serviceType);
						pdu.get_message().set_serviceType(serviceType);
					}
					break;
				case 2: //������ ����� �����������
					{
						__tc__("submitSm.assert.destAddrLengthInvalid");
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						addr.set_value("");
						pdu.get_message().set_dest(addr);
					}
					break;
				case 3: //destinationAddress ������ ������������ �����
					{
						__tc__("submitSm.assert.destAddrLengthInvalid");
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						char addrVal[MAX_ADDRESS_LENGTH + 10];
						rand_char(MAX_ADDRESS_LENGTH + 1, addrVal);
						addr.set_value(addrVal);
						pdu.get_message().set_dest(addr);
					}
					break;
				case 4: //message body ������ ������������ �����
					{
						__tc__("submitSm.assert.msgLengthInvalid");
						int len = MAX_SM_LENGTH + 1;
						auto_ptr<char> msg = rand_char(len);
						pdu.get_message().set_shortMessage(msg.get(), len);
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__dumpPdu__("submitSmAssert", fixture->smeInfo.systemId,
				reinterpret_cast<SmppHeader*>(&pdu));
			__tc_fail__(s.value());
		}
		catch (...)
		{
			__tc_ok__;
		}
	}
}

PduData* SmppProtocolTestCases::getNonReplaceEnrotePdu(bool deliveryReports)
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	//��� ���������� ������������ � ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			!m->pduData->replacePdu && m->getFlag() == PDU_REQUIRED_FLAG &&
			!m->pduData->intProps.count("hasSmppDuplicates") &&
			(deliveryReports ||
			 SmppTransmitterTestCases::getRegisteredDelivery(m->pduData) == NO_SMSC_DELIVERY_RECEIPT))
		{
			pduData = m->pduData;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getReplaceEnrotePdu(bool deliveryReports)
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	//��� ���������� � ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			m->pduData->replacePdu && m->getFlag() == PDU_REQUIRED_FLAG &&
			!m->pduData->intProps.count("hasSmppDuplicates") &&
			(deliveryReports ||
			 SmppTransmitterTestCases::getRegisteredDelivery(m->pduData) == NO_SMSC_DELIVERY_RECEIPT))
		{
			pduData = m->pduData;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getNonReplaceRescheduledEnrotePdu(bool deliveryReports)
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	//��� ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			!m->pduData->replacePdu && m->getFlag() == PDU_REQUIRED_FLAG &&
			!m->pduData->intProps.count("hasSmppDuplicates") &&
			(deliveryReports ||
			 SmppTransmitterTestCases::getRegisteredDelivery(m->pduData) == NO_SMSC_DELIVERY_RECEIPT))
		{
			DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
			__require__(monitor);
			//���� ������� ��������
			if (monitor->getLastTime())
			{
				pduData = monitor->pduData;
				break;
			}
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getFinalPdu(bool deliveryReports)
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		0, time(NULL) - timeCheckAccuracy);
	//��� ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			!m->pduData->replacePdu && m->getFlag() == PDU_NOT_EXPECTED_FLAG &&
			!m->pduData->intProps.count("hasSmppDuplicates") &&
			(deliveryReports ||
			 SmppTransmitterTestCases::getRegisteredDelivery(m->pduData) == NO_SMSC_DELIVERY_RECEIPT))
		{
			DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
			__require__(monitor);
			if (monitor->state == DELIVERED) //���� �������� ������� sms
			{
				pduData = monitor->pduData;
				break;
			}
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getPduByState(State state)
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduFlag flag;
	PduRegistry::PduMonitorIterator* it;
	if (state == ENROUTE)
	{
		flag = PDU_REQUIRED_FLAG;
		it = fixture->pduReg->getMonitors(time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	}
	else
	{
		flag = PDU_NOT_EXPECTED_FLAG;
		it = fixture->pduReg->getMonitors(0, time(NULL) - timeCheckAccuracy);
	}
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && m->getFlag() == flag)
		{
			DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
			__require__(monitor);
			if (monitor->state == state)
			{
				pduData = monitor->pduData;
				break;
			}
		}
	}
	delete it;
	return pduData;
}

void SmppProtocolTestCases::submitSmCorrect(bool sync, int num)
{
	TCSelector s(num, 11);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("submitSm.correct");
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(
				pdu, *destAlias, rand0(5));
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //������ ����������
					//__tc__("submitSm.correct");
					break;
				case 2: //������ serviceType
					__tc__("submitSm.correct.serviceTypeMarginal");
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					pdu->get_message().set_replaceIfPresentFlag(0);
					intProps["hasSmppDuplicates"] = 1;
					break;
				case 3: //serviceType ������������ �����
					{
						__tc__("submitSm.correct.serviceTypeMarginal");
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 4: //�������� ��� ������ ���� ��������
					{
						__tc__("submitSm.correct.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						//pdu->get_message().set_registredDelivery(0);
					}
					break;
				case 5: //���� ���������� ������ �������������
					{
						__tc__("submitSm.correct.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 6: //����������� ��������
					__tc__("submitSm.correct.waitTimeImmediate");
					pdu->get_message().set_scheduleDeliveryTime("");
					break;
				case 7: //���� ���������� �� ���������
					__tc__("submitSm.correct.validTimeDefault");
					pdu->get_message().set_validityPeriod("");
					break;
				case 8: //������ ���� ���������
					if (!pdu->get_optional().has_messagePayload())
					{
						__tc__("submitSm.correct.smLengthMarginal");
						pdu->get_message().set_shortMessage(NULL, 0);
						//pdu->get_message().set_shortMessage("", 0);
					}
					break;
				case 9: //shortMessage ������������ �����
					if (!pdu->get_optional().has_messagePayload())
					{
						__tc__("submitSm.correct.smLengthMarginal");
						auto_ptr<char> tmp = rand_char(MAX_SM_LENGTH);
						pdu->get_message().set_shortMessage(tmp.get(), MAX_SM_LENGTH);
					}
					break;
				case 10: //messagePayload ������������ �����
					if (!pdu->get_optional().has_messagePayload())
					{
						__tc__("submitSm.correct.messagePayloadLengthMarginal");
						auto_ptr<char> tmp = rand_char(MAX_PAYLOAD_LENGTH);
						pdu->get_optional().set_messagePayload(tmp.get(), MAX_PAYLOAD_LENGTH);
					}
					break;
				case 11: //ussd ������
					__tc__("submitSm.correct.ussdRequest");
					pdu->get_optional().set_ussdServiceOp(rand0(255));
					//����������� ��������, ussd �� �����������
					pdu->get_message().set_scheduleDeliveryTime("");
					//pdu->get_message().set_validityPeriod("");
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, &intProps);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::submitSmCorrectComplex(bool sync, int num)
{
	TCSelector s(num, 7);
	__decl_tc__;
	__tc__("submitSm.correct");
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(
				pdu, *destAlias, rand0(5));
			PduData* existentPduData = NULL;
			PduData::IntProps intProps;
			switch (s.value())
			{
				/*
				case 1: //msgRef ���������� (������������ msgRef �����������)
					//�������� GSM 03.40 ����� 9.2.3.25 ���� ���������
					//TP-MR, TP-DA, OA, �� ��� ETSI_REJECT_IF_PRESENT ����� ������.
					//��� SMPP ��� ������ �������� ���������� �� msgRef.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.checkMap");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							__require__(pdu->get_message().get_source() ==
								existentPdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_optional().set_userMessageReference(
								existentPdu->get_optional().get_userMessageReference());
						}
					}
					break;
				*/
				case 1: //�������� �������������� ��������� ��� ��������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.notReplace");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								nvl(existentPdu->get_message().get_serviceType()));
							__require__(pdu->get_message().get_source() ==
								existentPdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(0);
							intProps["hasSmppDuplicates"] = 1;
							existentPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				case 2: //�������� �������������� ��������� � �������� ���������
					//��� �������������, �� service_type �� ���������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.serviceTypeNotMatch");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							__require__(strcmp(nvl(pdu->get_message().get_serviceType()),
								nvl(existentPdu->get_message().get_serviceType())));
							//pdu->get_message().set_serviceType(...);
							__require__(pdu->get_message().get_source() ==
								existentPdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 3: //�������� �������������� ��������� � �������� ���������
					//��� �������������, �� dest_addr �� ���������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							if (pdu->get_message().get_dest() !=
								existentPdu->get_message().get_dest())
							{
								__tc__("submitSm.correct.destAddrNotMatch");
								pdu->get_message().set_serviceType(
									nvl(existentPdu->get_message().get_serviceType()));
								__require__(pdu->get_message().get_source() ==
									existentPdu->get_message().get_source());
								//pdu->get_message().set_source(...);
								__require__(pdu->get_message().get_dest() !=
									existentPdu->get_message().get_dest());
								//pdu->get_message().set_dest(...);
								pdu->get_message().set_replaceIfPresentFlag(1);
							}
						}
					}
					break;
				case 4: //�������� �������������� ��������� � ���������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								nvl(replacePdu->get_message().get_serviceType()));
							__require__(pdu->get_message().get_source() ==
								replacePdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 5: //�������� �������������� ��������� � ���������� ���
					//����� �����������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceReplacedEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								nvl(replacePdu->get_message().get_serviceType()));
							__require__(pdu->get_message().get_source() ==
								replacePdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 6: //�������� �������������� ��������� � ���������� ���
					//�������������, �� ������������ ��� � ��������� ���������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = getFinalPdu(true);
						if (finalPduData)
						{
							__require__(finalPduData->pdu &&
								finalPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceFinal");
							PduSubmitSm* finalPdu =
								reinterpret_cast<PduSubmitSm*>(finalPduData->pdu);
							pdu->get_message().set_serviceType(
								nvl(finalPdu->get_message().get_serviceType()));
							__require__(pdu->get_message().get_source() ==
								finalPdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								finalPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
							intProps["hasSmppDuplicates"] = 1;
							finalPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				case 7: //�������� �������������� ��������� � ���������� ���
					//�������������, �� ������������ ��� � �������� ��������� ��������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceRescheduledEnrotePdu(true);
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceRepeatedDeliveryEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								nvl(replacePdu->get_message().get_serviceType()));
							__require__(pdu->get_message().get_source() ==
								replacePdu->get_message().get_source());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, existentPduData, sync, &intProps);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::submitSmIncorrect(bool sync, int num)
{
	TCSelector s(num, 13 /*19*/);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("submitSm.incorrect");
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, *destAlias, rand0(5));
			switch (s.value())
			{
				case 1: //������������ ����� �����������
					{
						__tc__("submitSm.incorrect.sourceAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->get_message().set_source(smppAddr);
					}
					break;
				case 2: //������������ ����� ����������
					{
						__tc__("submitSm.incorrect.destAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->get_message().set_dest(smppAddr);
					}
					break;
				case 3: //������������ ������ validity_period
					{
						__tc__("submitSm.incorrect.validTimeFormat");
						string t = pdu->get_message().get_validityPeriod();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->get_message().set_validityPeriod(t.c_str());
					}
					break;
				case 4: //������������ ������ validity_period
					{
						__tc__("submitSm.incorrect.validTimeFormat");
						string t = pdu->get_message().get_validityPeriod();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->get_message().set_validityPeriod(t.c_str());
					}
					break;
				case 5: //������������ ������ schedule_delivery_time
					{
						__tc__("submitSm.incorrect.waitTimeFormat");
						string t = pdu->get_message().get_scheduleDeliveryTime();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->get_message().set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 6: //������������ ������ schedule_delivery_time
					{
						__tc__("submitSm.incorrect.waitTimeFormat");
						string t = pdu->get_message().get_scheduleDeliveryTime();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->get_message().set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 7: //���� ���������� ��� ����������
					{
						__tc__("submitSm.incorrect.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - timeCheckAccuracy, t, time(NULL), __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 8: //waitTime > validTime
					{
						__tc__("submitSm.incorrect.waitTimeInvalid1");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(maxWaitTime);
						time_t waitTime = validTime + timeCheckAccuracy;
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 9: //waitTime > validTime
					{
						__tc__("submitSm.incorrect.waitTimeInvalid2");
						SmppTime t;
						time_t waitTime = time(NULL) + maxValidPeriod + timeCheckAccuracy;
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						pdu->get_message().set_validityPeriod("");
					}
					break;
				case 10: //waitTime > validTime
					{
						__tc__("submitSm.incorrect.waitTimeInvalid3");
						SmppTime t;
						time_t waitTime = time(NULL) + maxValidPeriod + timeCheckAccuracy;
						time_t validTime = waitTime + rand1(maxDeliveryPeriod);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 11: //������������ dataCoding
					__tc__("submitSm.incorrect.dataCoding");
					for (int dc = rand1(255); true; dc = rand1(255))
					{
						switch (dc)
						{
							case DEFAULT:
							case BINARY:
							case UCS2:
							case SMSC7BIT:
								continue;
						}
						pdu->get_message().set_dataCoding(dc);
						break;
					}
					break;
				case 12: //������ ��� short_message � message_payload
					__tc__("submitSm.incorrect.bothMessageFields");
					if (!pdu->get_message().size_shortMessage())
					{
						int len = rand1(MAX_SM_LENGTH);
						auto_ptr<char> tmp = rand_char(len);
						pdu->get_message().set_shortMessage(tmp.get(), len);
					}
					if (!pdu->get_optional().has_messagePayload())
					{
						int len = rand1(MAX_PAYLOAD_LENGTH);
						auto_ptr<char> tmp = rand_char(len);
						pdu->get_optional().set_messagePayload(tmp.get(), len);
					}
					break;
				case 13:
					__tc__("submitSm.incorrect.transactionRollback");
					pdu->get_message().set_serviceType("-----");
					break;
				/*
				case 13: //������������ ����� udh
					__tc__("submitSm.incorrect.udhiLength");
					pdu->get_message().set_esmClass(
						pdu->get_message().get_esmClass() | ESM_CLASS_UDHI_INDICATOR);
					{
						int len = rand1(5);
						char buf[len];
						*buf = (unsigned char) (len + rand1(10));
						pdu->get_message().set_shortMessage(buf, len);
					}
					break;
				case 13: //����� service_type ������ ����������
					{
						__tc__("submitSm.incorrect.serviceTypeLength");
						auto_ptr<char> tmp = rand_char(MAX_SERVICE_TYPE_LENGTH + 1);
						pdu->get_message().set_serviceType(tmp.get());
					}
					break;
				case 14: //����� source_addr ������ ����������
					{
						__tc__("submitSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->get_message().set_source(addr);
					}
					break;
				case 15: //����� dest_addr ������ ����������
					{
						__tc__("submitSm.incorrect.destAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->get_message().set_dest(addr);
					}
					break;
				case 16: //����� validity_period ������ ����������
					{
						__tc__("submitSm.incorrect.validTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->get_message().set_validityPeriod(tmp.get());
					}
					break;
				case 17: //����� schedule_delivery_time ������ ����������
					{
						__tc__("submitSm.incorrect.waitTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->get_message().set_scheduleDeliveryTime(tmp.get());
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::replaceSmCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	TCSelector s(num, 9);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("replaceSm.correct");
	for (; s.check(); s++)
	{
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			//������� ��������� messageId �� sms ��������� ��������
			PduData* replacePduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu(false);
			}
			if (!replacePduData)
			{
				__tc__("replaceSm.incorrect.messageId");
				SmppUtil::setupRandomCorrectReplaceSmPdu(pdu, DEFAULT, false);
				PduAddress srcAddr;
				SmppUtil::convert(fixture->smeAddr, &srcAddr);
				pdu->set_source(srcAddr);
				//��������� � ���������������� pdu
				fixture->transmitter->sendReplaceSmPdu(pdu, NULL, sync);
				__tc_ok__;
				return;
			}
			fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu, replacePduData);
			//replaced pdu params
			__require__(replacePduData && replacePduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* replacePdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
			time_t waitTime = SmppUtil::getWaitTime(
				replacePdu->get_message().get_scheduleDeliveryTime(), replacePduData->sendTime);
			time_t validTime = SmppUtil::getValidTime(
				replacePdu->get_message().get_validityPeriod(), replacePduData->sendTime);
			switch (s.value())
			{
				case 1: //������ ����������
					//__tc__("replaceSm.correct");
					break;
				case 2: //�������� ��� ������ ���� ��������
					{
						__tc__("replaceSm.correct.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						//pdu->set_registredDelivery(0);
					}
					break;
				case 3: //���� ���������� ������ �������������
					{
						__tc__("replaceSm.correct.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 4: //����� �������� ��� ���������
					{
						__tc__("replaceSm.correct.waitTimeNull");
						SmppTime t;
						SmppUtil::time2string(
							max(waitTime, time(NULL)) + timeCheckAccuracy,
							t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime("");
						pdu->set_validityPeriod(t);
					}
					break;
				case 5: //���� ���������� ��� ���������
					{
						__tc__("replaceSm.correct.validTimeNull");
						SmppTime t;
						SmppUtil::time2string(validTime - timeCheckAccuracy, t,
							time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime(t);
						pdu->set_validityPeriod("");
					}
					break;
				case 6: //������ ���� ���������
					__tc__("replaceSm.correct.smLengthMarginal");
					pdu->set_shortMessage(NULL, 0);
					//pdu->set_shortMessage("", 0);
					break;
				case 7: //���� ��������� ������������ �����
					{
						__tc__("replaceSm.correct.smLengthMarginal");
						auto_ptr<char> msg = rand_char(MAX_SM_LENGTH);
						pdu->set_shortMessage(msg.get(), MAX_SM_LENGTH);
					}
					break;
				case 8: //��������� ��� ����� ����������� ���������
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu(false);
						if (replacePduData)
						{
							__tc__("replaceSm.correct.replaceReplacedEnrote");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(
								pdu, replacePduData);
						}
					}
					break;
				case 9: //��������� ������������� ���������, �� ������������
					//��� � �������� ��������� ��������.
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu(false);
						if (replacePduData)
						{
							__tc__("replaceSm.correct.replaceRepeatedDeliveryEnrote");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(
								pdu, replacePduData);
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::replaceSmIncorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	TCSelector s(num, 13 /*17*/);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(timeCheckAccuracy);
	__tc__("replaceSm.correct"); //correct ���� �� ����������� case
	for (; s.check(); s++)
	{
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			//������� ��������� sms ��������� ��������
			PduData* replacePduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu(false);
			}
			if (!replacePduData)
			{
				__tc__("replaceSm.incorrect.messageId");
				SmppUtil::setupRandomCorrectReplaceSmPdu(pdu, DEFAULT, false);
				PduAddress srcAddr;
				SmppUtil::convert(fixture->smeAddr, &srcAddr);
				pdu->set_source(srcAddr);
				//��������� � ���������������� pdu
				fixture->transmitter->sendReplaceSmPdu(pdu, NULL, sync);
				__tc_ok__;
				return;
			}
			fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu, replacePduData);
			//replaced pdu params
			__require__(replacePduData && replacePduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* replacePdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
			time_t waitTime = SmppUtil::getWaitTime(
				replacePdu->get_message().get_scheduleDeliveryTime(), replacePduData->sendTime);
			time_t validTime = SmppUtil::getValidTime(
				replacePdu->get_message().get_validityPeriod(), replacePduData->sendTime);
			switch (s.value())
			{
				case 1: //������������ message_id
					{
						__tc__("replaceSm.incorrect.messageId");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
					}
					break;
				case 2: //������������ message_id
					{
						__tc__("replaceSm.incorrect.messageId");
						pdu->set_messageId("-1");
					}
					break;
				case 3: //������������ message_id
					{
						__tc__("replaceSm.incorrect.messageId");
						pdu->set_messageId("36893488147419103232"); //2**65
					}
					break;
				case 4: //������������ ����� �����������
					{
						__tc__("replaceSm.incorrect.sourceAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_source(smppAddr);
					}
					break;
				case 5: //������������ ������ validity_period
					{
						__tc__("replaceSm.incorrect.validTimeFormat");
						string t = pdu->get_validityPeriod();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->set_validityPeriod(t.c_str());
					}
					break;
				case 6: //������������ ������ validity_period
					{
						__tc__("replaceSm.incorrect.validTimeFormat");
						string t = pdu->get_validityPeriod();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->set_validityPeriod(t.c_str());
					}
					break;
				case 7: //������������ ������ schedule_delivery_time
					{
						__tc__("replaceSm.incorrect.waitTimeFormat");
						string t = pdu->get_scheduleDeliveryTime();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 8: //������������ ������ schedule_delivery_time
					{
						__tc__("replaceSm.incorrect.waitTimeFormat");
						string t = pdu->get_scheduleDeliveryTime();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 9: //���� ���������� ��� ����������
					{
						__tc__("replaceSm.incorrect.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - timeCheckAccuracy, t, time(NULL), __absoluteTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 10: //waitTime > validTime
					{
						__tc__("replaceSm.incorrect.waitTimeInvalid1");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(maxWaitTime);
						time_t waitTime = validTime + timeCheckAccuracy;
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 11: //waitTime > validTime
					{
						__tc__("replaceSm.incorrect.waitTimeInvalid2");
						SmppTime t;
						time_t newWaitTime = validTime + timeCheckAccuracy;
						SmppUtil::time2string(newWaitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						pdu->set_validityPeriod("");
					}
					break;
				case 12: //waitTime > validTime
					{
						__tc__("replaceSm.incorrect.waitTimeInvalid3");
						SmppTime t;
						time_t newValidTime = waitTime - timeCheckAccuracy;
						SmppUtil::time2string(
							newValidTime, t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime("");
						pdu->set_validityPeriod(t);
					}
					break;
				case 13: //���������� ������������� ���������, �� ������������
					//��� � ��������� ���������.
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = getFinalPdu(false);
						if (finalPduData)
						{
							__tc__("replaceSm.incorrect.replaceFinal");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(
								pdu, finalPduData);
						}
					}
					break;
				/*
				case 14: //����� message_id ������ ����������
					{
						__tc__("replaceSm.incorrect.messageIdLength");
						auto_ptr<char> tmp = rand_char(MAX_MSG_ID_LENGTH + 1);
						pdu->set_messageId(tmp.get());
					}
					break;
				case 15: //����� source_addr ������ ����������
					{
						__tc__("replaceSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_source(addr);
					}
					break;
				case 16: //����� validity_period ������ ����������
					{
						__tc__("replaceSm.incorrect.validTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->set_validityPeriod(tmp.get());
					}
					break;
				case 17: //����� schedule_delivery_time ������ ����������
					{
						__tc__("replaceSm.incorrect.waitTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->set_scheduleDeliveryTime(tmp.get());
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::querySmCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 5);
	__tc__("querySm.correct");
	for (; s.check(); s++)
	{
		try
		{
			PduQuerySm* pdu = new PduQuerySm();
			//������� ��������� messageId �� sms ��������� ��������
			PduData* pduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				switch (s.value())
				{
					case 1:
						__tc__("querySm.correct.enroute");
						pduData = getPduByState(ENROUTE);
						break;
					case 2:
						__tc__("querySm.correct.delivered");
						pduData = getPduByState(DELIVERED);
						break;
					case 3:
						__tc__("querySm.correct.expired");
						pduData = getPduByState(EXPIRED);
						break;
					case 4:
						__tc__("querySm.correct.undeliverable");
						pduData = getPduByState(UNDELIVERABLE);
						break;
					case 5:
						__tc__("querySm.correct.deleted");
						pduData = getPduByState(DELETED);
						break;
					default:
						__unreachable__("Invalid num state");
				}
			}
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			if (pduData)
			{
				__require__(pduData->strProps.count("smsId"));
				pdu->set_messageId(pduData->strProps["smsId"].c_str());
			}
			else
			{
				__tc__("querySm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendQuerySmPdu(pdu, pduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::querySmIncorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	int numState = 5; int numOther = 3 /*5*/;
	TCSelector s(num, numState * numOther);
	__tc__("querySm.incorrect");
	for (; s.check(); s++)
	{
		try
		{
			PduQuerySm* pdu = new PduQuerySm();
			//������� ��������� messageId �� sms ��������� ��������
			PduData* pduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				switch (s.value1(numState))
				{
					case 1:
						pduData = getPduByState(ENROUTE);
						break;
					case 2:
						pduData = getPduByState(DELIVERED);
						break;
					case 3:
						pduData = getPduByState(EXPIRED);
						break;
					case 4:
						pduData = getPduByState(UNDELIVERABLE);
						break;
					case 5:
						pduData = getPduByState(DELETED);
						break;
					default:
						__unreachable__("Invalid num state");
				}
			}
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			switch (s.value2(numState))
			{
				case 1: //������������ messageId
					__tc__("querySm.incorrect.messageId");
					pdu->set_messageId("-1");
					break;
				case 2: //������������ messageId
					__tc__("querySm.incorrect.messageId");
					pdu->set_messageId("36893488147419103232"); // 2**65
					break;
				case 3: //�� ��������� srcAddr
					if (pduData)
					{
						__tc__("querySm.incorrect.sourceAddr");
						__require__(pduData->strProps.count("smsId"));
						pdu->set_messageId(pduData->strProps["smsId"].c_str());
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_source(smppAddr);
					}
					else
					{
						__tc__("querySm.incorrect.messageId");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
					}
					break;
				/*
				case 4: //����� message_id ������ ����������
					{
						__tc__("querySm.incorrect.messageIdLength");
						auto_ptr<char> tmp = rand_char(MAX_MSG_ID_LENGTH + 1);
						pdu->set_messageId(tmp.get());
					}
					break;
				case 5: //����� source_addr ������ ����������
					{
						__tc__("querySm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_source(addr);
					}
					break;
				*/
				default:
					__unreachable__("Invalid num other");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendQuerySmPdu(pdu, pduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::cancelSmCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 4);
	__tc__("cancelSm.correct");
	for (; s.check(); s++)
	{
		try
		{
			PduCancelSm* pdu = new PduCancelSm();
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			//������� ��������� sms ��������� ��������
			PduData* cancelPduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				cancelPduData = getPduByState(ENROUTE);
			}
			if (!cancelPduData)
			{
				__tc__("cancelSm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
				//pdu->set_source();
				//pdu->set_dest();
				//pdu->set_serviceType();
				//��������� � ���������������� pdu
				fixture->transmitter->sendCancelSmPdu(pdu, NULL, sync);
				__tc_ok__;
				return;
			}
			__trace2__("cancelPduData = %p", cancelPduData);
			//canceld pdu params
			__require__(cancelPduData->strProps.count("smsId"));
			__require__(cancelPduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* cancelPdu = reinterpret_cast<PduSubmitSm*>(cancelPduData->pdu);
			__require__(cancelPdu->get_message().get_source() == srcAddr);
			switch (s.value())
			{
				case 1: //�� message_id � source_addr � �������� service_type, dest_addr
					__tc__("cancelSm.correct.messageIdWithoutDestAddr");
					pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 2: //�� message_id, source_addr � dest_addr � ������� service_type
					__tc__("cancelSm.correct.messageIdWithDestAddr");
					pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
					//pdu->set_source();
					pdu->set_dest(cancelPdu->get_message().get_dest());
					//pdu->set_serviceType();
					break;
				case 3: //�� source_addr � dest_addr � ������� service_type
					__tc__("cancelSm.correct.destAddrWithoutServiceType");
					//pdu->set_messageId();
					//pdu->set_source();
					pdu->set_dest(cancelPdu->get_message().get_dest());
					//pdu->set_serviceType();
					break;
				case 4: //�� source_addr, dest_addr � service_type
					__tc__("cancelSm.correct.destAddrWithServiceType");
					//pdu->set_messageId();
					//pdu->set_source();
					pdu->set_dest(cancelPdu->get_message().get_dest());
					pdu->set_serviceType(
						nvl(cancelPdu->get_message().get_serviceType()));
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendCancelSmPdu(pdu, cancelPduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::cancelSmIncorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 15 /*19*/);
	__tc__("cancelSm.incorrect");
	for (; s.check(); s++)
	{
		try
		{
			static /* const */ PduAddress nullAddr;
			PduCancelSm* pdu = new PduCancelSm();
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			//������� ��������� sms ��������� ��������
			PduData* cancelPduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				cancelPduData = getPduByState(ENROUTE);
			}
			if (!cancelPduData)
			{
				__tc__("cancelSm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
				//pdu->set_source();
				//pdu->set_dest();
				//pdu->set_serviceType();
				//��������� � ���������������� pdu
				fixture->transmitter->sendCancelSmPdu(pdu, NULL, sync);
				__tc_ok__;
				return;
			}
			//canceld pdu params
			__require__(cancelPduData->strProps.count("smsId"));
			__require__(cancelPduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* cancelPdu = reinterpret_cast<PduSubmitSm*>(cancelPduData->pdu);
			switch (s.value())
			{
				case 1: //������������ message_id
					{
						__tc__("cancelSm.incorrect.messageId");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
						//pdu->set_source();
						//pdu->set_dest();
						//pdu->set_serviceType();
					}
					break;
				case 2: //������������ message_id
					__tc__("cancelSm.incorrect.messageId");
					pdu->set_messageId("-1");
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 3: //������������ message_id
					__tc__("cancelSm.incorrect.messageId");
					pdu->set_messageId("36893488147419103232"); //2**65
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 4: //����� ����������� �� ���������
					{
						__tc__("cancelSm.incorrect.sourceAddr");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_source(smppAddr);
						//pdu->set_dest();
						//pdu->set_serviceType();
					}
					break;
				case 5: //����� ���������� �� ���������
					{
						__tc__("cancelSm.incorrect.destAddr");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						//pdu->set_source();
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_dest(smppAddr);
						//pdu->set_serviceType();
					}
					break;
				case 6: //����� ������ source_addr
					__tc__("cancelSm.incorrect.sourceAddrOnly");
					//pdu->set_messageId();
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 7: //����� ������ dest_addr
					{
						__tc__("cancelSm.incorrect.destAddrOnly");
						//pdu->set_messageId();
						pdu->set_source(nullAddr);
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_dest(smppAddr);
						//pdu->set_serviceType();
					}
					break;
				case 8: //����� ������ message_id
					__tc__("cancelSm.incorrect.messageIdOnly");
					pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
					pdu->set_source(nullAddr);
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 9: //��� ���� ������
					__tc__("cancelSm.incorrect.emptyFields");
					//pdu->set_messageId();
					pdu->set_source(nullAddr);
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 10: //����� service_type ��� dest_addr
					{
						__tc__("cancelSm.incorrect.serviceTypeWithoutDestAddr");
						//pdu->set_messageId();
						//pdu->set_source();
						//pdu->set_dest();
						auto_ptr<char> tmp = rand_char(MAX_ESERVICE_TYPE_LENGTH);
						pdu->set_serviceType(tmp.get());
					}
					break;
				case 11: //������ ���������� message_id � service_type
					{
						__tc__("cancelSm.incorrect.messageIdWithServiceType");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						//pdu->set_source();
						//pdu->set_dest();
						pdu->set_serviceType(
							nvl(cancelPdu->get_message().get_serviceType()));
					}
					break;
				case 12: //������ ���������� message_id, dest_addr � service_type
					{
						__tc__("cancelSm.incorrect.allFields");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						//pdu->set_source();
						pdu->set_dest(cancelPdu->get_message().get_dest());
						pdu->set_serviceType(
							nvl(cancelPdu->get_message().get_serviceType()));
					}
					break;
				case 13: //��������� � ��������� ���������
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						cancelPduData = getFinalPdu(true);
						if (cancelPduData)
						{
							__tc__("cancelSm.incorrect.cancelFinal");
							__require__(cancelPduData->strProps.count("smsId"));
							pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
							//pdu->set_source();
							//pdu->set_dest();
							//pdu->set_serviceType();
						}
					}
					break;
				case 14: //������ sms-�� ������
					{
						__tc__("cancelSm.incorrect.noSms");
						//pdu->set_messageId();
						//pdu->set_source();
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_dest(smppAddr);
						//pdu->set_serviceType();
					}
					break;
				case 15: //������ sms-�� ������
					{
						__tc__("cancelSm.incorrect.noSms");
						//pdu->set_messageId();
						//pdu->set_source();
						pdu->set_dest(cancelPdu->get_message().get_dest());
						auto_ptr<char> tmp = rand_char(MAX_ESERVICE_TYPE_LENGTH);
						pdu->set_serviceType(tmp.get());
					}
					break;
				/*
				case 16: //����� message_id ������ ����������
					{
						__tc__("cancelSm.incorrect.messageIdLength");
						auto_ptr<char> tmp = rand_char(MAX_MSG_ID_LENGTH + 1);
						pdu->set_messageId(tmp.get());
					}
					break;
				case 17: //����� service_type ������ ����������
					{
						__tc__("cancelSm.incorrect.serviceTypeLength");
						auto_ptr<char> tmp = rand_char(MAX_SERVICE_TYPE_LENGTH + 1);
						pdu->set_serviceType(tmp.get());
					}
					break;
				case 18: //����� source_addr ������ ����������
					{
						__tc__("cancelSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_source(addr);
					}
					break;
				case 19: //����� dest_addr ������ ����������
					{
						__tc__("cancelSm.incorrect.destAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_dest(addr);
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			fixture->transmitter->sendCancelSmPdu(pdu, cancelPduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::sendInvalidPdu(bool sync, int num)
{
	__decl_tc__;
	TCSelector s(num, 14);
	for (; s.check(); s++)
	{
		try
		{
			SmppHeader* pdu;
			switch (s.value())
			{
				//��������
				case 1:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduQuerySm());
					pdu->set_commandId(QUERY_SM);
					break;
				case 2:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduDeliverySm());
					pdu->set_commandId(DELIVERY_SM);
					break;
				case 3:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduCancelSm());
					pdu->set_commandId(CANCEL_SM);
					break;
				case 4:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduOutBind());
					pdu->set_commandId(OUTBIND);
					break;
				case 5:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduMultiSm());
					pdu->set_commandId(SUBMIT_MULTI);
					break;
				case 6:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduAlertNotification());
					pdu->set_commandId(ALERT_NOTIFICATION);
					break;
				case 7:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduDataSm());
					pdu->set_commandId(DATA_SM);
					break;
				//��������
				case 8:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
					pdu->set_commandId(BIND_RECIEVER_RESP);
					break;
				case 9:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
					pdu->set_commandId(BIND_TRANSMITTER_RESP);
					break;
				case 10:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
					pdu->set_commandId(BIND_TRANCIEVER_RESP);
					break;
				case 11:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduQuerySmResp());
					pdu->set_commandId(QUERY_SM_RESP);
					break;
				case 12:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduSubmitSmResp());
					pdu->set_commandId(SUBMIT_SM_RESP);
					break;
				case 13:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduCancelSmResp());
					pdu->set_commandId(CANCEL_SM_RESP);
					break;
				case 14:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduDataSmResp());
					pdu->set_commandId(DATA_SM_RESP);
					break;
				/*
				case 15:
					__tc__("sendInvalidPdu.response");
					{
						PduMultiSmResp* p = new PduMultiSmResp();
						p->set_sme(NULL);
						pdu = reinterpret_cast<SmppHeader*>(p);
					}
					pdu->set_commandId(SUBMIT_MULTI_RESP);
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			pdu->set_commandStatus(rand0(1));
			//��������� � ���������������� pdu
			fixture->transmitter->sendInvalidPdu(pdu, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

int SmppProtocolTestCases::getRandomRespDelay()
{
	__cfg_int__(timeCheckAccuracy);
	return 1000 * rand0(max(0, (int) fixture->smeInfo.timeout - timeCheckAccuracy));
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespOk(
	PduDeliverySm& pdu, bool sync, bool sendDelay)
{
	__decl_tc__;
	int delay = 0;
	time_t sendTime = time(NULL);
	if (sendDelay)
	{
		__tc__("deliverySm.resp.delay"); __tc_ok__;
		delay = getRandomRespDelay();
		sendTime += delay / 1000;
	}
	__trace2__("sendDeliverySmRespOk(): delay = %d", delay);
	try
	{
		__tc__("deliverySm.resp.sendOk");
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
		fixture->transmitter->sendDeliverySmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(ESME_ROK, sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(DELIVERY_STATUS_NO_RESPONSE, time(NULL));
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespRetry(
	PduDeliverySm& pdu, bool sync, int num)
{
	__trace2__("sendDeliverySmRespRetry(): sme timeout = %d", fixture->smeInfo.timeout);
	TCSelector s(num, 5);
	__decl_tc__;
	try
	{
		uint32_t commandStatus = ESME_ROK;
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK);
		switch (s.value())
		{
			case 1: //�� ���������� �������
				__tc__("deliverySm.resp.sendRetry.notSend");
				commandStatus = DELIVERY_STATUS_NO_RESPONSE;
				break;
			case 2: //��������� ������ �� ������� sme, ������ �� ��������� ��������
				__tc__("deliverySm.resp.sendRetry.tempAppError");
				commandStatus = ESME_RX_T_APPN;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 3: //������������ ������� ������� sme
				__tc__("deliverySm.resp.sendRetry.msgQueueFull");
				commandStatus = ESME_RMSGQFUL;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 4: //��������� ������� � ������������ sequence_number
				__tc__("deliverySm.resp.sendRetry.invalidSequenceNumber");
				commandStatus = DELIVERY_STATUS_NO_RESPONSE;
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(ESME_ROK);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 5: //��������� ������� ����� sme timeout
				{
					__tc__("deliverySm.resp.sendRetry.sendAfterSmeTimeout");
					commandStatus = DELIVERY_STATUS_NO_RESPONSE;
					respPdu.get_header().set_commandStatus(ESME_ROK);
					int timeout = 1000 * (fixture->smeInfo.timeout + 1);
					__trace2__("sendAfterSmeTimeout(): timeout = %d", timeout);
					fixture->transmitter->sendDeliverySmResp(respPdu, sync, timeout);
				}
				break;
			default:
				__unreachable__("Invalid num");
		}
		__tc_ok__;
		return make_pair(commandStatus, time(NULL));
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return make_pair(DELIVERY_STATUS_NO_RESPONSE, time(NULL));
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespError(
	PduDeliverySm& pdu, bool sync, bool sendDelay, int num)
{
	__decl_tc__;
	int delay = 0;
	time_t sendTime = time(NULL);
	if (sendDelay)
	{
		__tc__("deliverySm.resp.delay"); __tc_ok__;
		delay = getRandomRespDelay();
		sendTime += delay / 1000;
	}
	__trace2__("sendDeliverySmRespError(): delay = %d", delay);
	TCSelector s(num, 3);
	try
	{
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		switch (s.value())
		{
			case 1: //��������� ������� � ����� ������ 0x1-0x10f
				__tc__("deliverySm.resp.sendError.standardError");
				respPdu.get_header().set_commandStatus(rand1(0x10f));
				break;
			case 2: //��������� ������� � ����� ������:
				//0x110-0x3ff - Reserved for SMPP extension
				//0x400-0x4ff - Reserved for SMSC vendor specific
				__tc__("deliverySm.resp.sendError.reservedError");
				respPdu.get_header().set_commandStatus(rand2(0x110, 0x4ff));
				break;
			case 3: //��������� ������� � ����� ������ >0x500 - Reserved
				__tc__("deliverySm.resp.sendError.outRangeError");
				respPdu.get_header().set_commandStatus(rand2(0x500, INT_MAX));
				break;
			/*
			case 4: //������������ ������ �� ������� sme, ����� �� ���� ����������� ���������
				__tc__("deliverySm.resp.sendError.permanentAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_P_APPN);
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		//����������� �����, ����� ������������� �������� smpp ������
		if (respPdu.get_header().get_commandStatus() == ESME_RX_P_APPN)
		{
			return sendDeliverySmRespError(pdu, sync, sendDelay, num);
		}
		fixture->transmitter->sendDeliverySmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(respPdu.get_header().get_commandStatus(), sendTime);
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return make_pair(DELIVERY_STATUS_NO_RESPONSE, time(NULL));
}

}
}
}

