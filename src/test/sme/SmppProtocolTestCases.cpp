#include "SmppProtocolTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::TestConfig;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::sms; //constants
using namespace smsc::test::smpp;
using namespace smsc::test::core; //constants
using namespace smsc::test::util;

SmppProtocolTestCases::SmppProtocolTestCases(const SmeConfig& config,
	SmppFixture* _fixture) : fixture(_fixture), chkList(fixture->chkList)
{
	__require__(fixture);
	base = new SmppBaseTestCases(config, fixture);
	receiver = new SmppReceiverTestCases(fixture);
	transmitter = new SmppTransmitterTestCases(fixture);
	fixture->session = new SmppSession(config, receiver);
}

SmppProtocolTestCases::~SmppProtocolTestCases()
{
	delete base;
	delete receiver;
	delete transmitter;
	delete fixture;
}

Category& SmppProtocolTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppProtocolTestCases");
	return log;
}

SmppBaseTestCases& SmppProtocolTestCases::getBase()
{
	return *base;
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
			__dumpSubmitSmPdu__("SmppProtocolTestCases::submitSmAssert", fixture->systemId, &pdu);
			__tc_fail__(s.value());
		}
		catch (...)
		{
			__tc_ok__;
		}
	}
}

PduData* SmppProtocolTestCases::getNonReplaceEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduDataIterator* it = fixture->pduReg->getPduByWaitTime(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//��� ���������� ������������ � ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && !data->replacePdu &&
			data->smsId.length() && data->deliveryFlag == PDU_REQUIRED_FLAG &&
			!data->intProps.size() && !data->strProps.size())
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getReplaceEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduDataIterator* it = fixture->pduReg->getPduByWaitTime(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//��� ���������� � ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && data->replacePdu &&
			data->smsId.length() && data->deliveryFlag == PDU_REQUIRED_FLAG &&
			!data->intProps.size() && !data->strProps.size())
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getNonReplaceRescheduledEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduDataIterator* it = fixture->pduReg->getPduByWaitTime(0, time(NULL));
	//��� ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && data->smsId.length() &&
			data->deliveryFlag == PDU_REQUIRED_FLAG &&
			!data->intProps.size() && !data->strProps.size() &&
			data->deliveryFlag.getNextTime(time(NULL)) >=
			time(NULL) + sequentialPduInterval)
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

void SmppProtocolTestCases::submitSmCorrect(bool sync, int num)
{
	TCSelector s(num, 15);
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
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, *destAlias);
			PduData* existentPduData = NULL;
			switch (s.value())
			{
				case 1: //������ ����������
					//__tc__("submitSm.correct");
					break;
				/*
				case 2: //������ serviceType
					__tc__("submitSm.correct.serviceTypeMarginal");
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					break;
				*/
				case 2: //serviceType ������������ �����
					{
						__tc__("submitSm.correct.serviceTypeMarginal");
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 3: //�������� ��� ������ ���� ��������
					{
						__tc__("submitSm.correct.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						pdu->get_message().set_registredDelivery(0);
					}
					break;
				case 4: //���� ���������� ������ �������������
					{
						__tc__("submitSm.correct.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 5: //����������� ��������
					__tc__("submitSm.correct.waitTimeImmediate");
					pdu->get_message().set_scheduleDeliveryTime("");
					break;
				case 6: //���� ���������� �� ���������
					__tc__("submitSm.correct.validTimeDefault");
					pdu->get_message().set_validityPeriod("");
					break;
				case 7: //������ ���� ���������
					__tc__("submitSm.correct.smLengthMarginal");
					pdu->get_message().set_shortMessage(NULL, 0);
					//pdu->get_message().set_shortMessage("", 0);
					break;
				case 8: //���� ��������� ������������ �����
					{
						__tc__("submitSm.correct.smLengthMarginal");
						ShortMessage msg;
						rand_char(MAX_SM_LENGTH, msg);
						pdu->get_message().set_shortMessage(msg, MAX_SM_LENGTH);
					}
					break;
				case 9: //msgRef ���������� (������������ msgRef �����������)
					//�������� GSM 03.40 ����� 9.2.3.25 ���� ���������
					//TP-MR, TP-DA, OA, �� ��� ETSI_REJECT_IF_PRESENT ����� ������.
					//��� SMPP ��� ������ �������� ���������� �� msgRef.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.checkMap");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_optional().set_userMessageReference(
								existentPdu->get_optional().get_userMessageReference());
						}
					}
					break;
				case 10: //�������� �������������� ��������� ��� ��������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.notReplace");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								existentPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(0);
						}
					}
					break;
				case 11: //�������� �������������� ��������� � �������� ���������
					//��� �������������, �� service_type �� ���������.
					//��� ������������� source address � destination address
					//���� ������ �� �����, �.�. ��������� �������� � ����������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.serviceTypeNotMatch");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							//pdu->get_message().set_serviceType(...);
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 12: //�������� �������������� ��������� � ���������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 13: //�������� �������������� ��������� � ���������� ���
					//����� �����������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceReplacedEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 14: //�������� �������������� ��������� � ���������� ���
					//�������������, �� ������������ ��� � ��������� ���������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = fixture->pduReg->getLastRemovedPdu();
						if (finalPduData)
						{
							__require__(finalPduData->pdu &&
								finalPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceFinal");
							PduSubmitSm* finalPdu =
								reinterpret_cast<PduSubmitSm*>(finalPduData->pdu);
							pdu->get_message().set_serviceType(
								finalPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								finalPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 15: //�������� �������������� ��������� � ���������� ���
					//�������������, �� ������������ ��� � �������� ��������� ��������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceRescheduledEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceRepeatedDeliveryEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
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
			transmitter->sendSubmitSmPdu(pdu, existentPduData, sync);
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
	TCSelector s(num, 10);
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
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, *destAlias);
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
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			transmitter->sendSubmitSmPdu(pdu, NULL, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::replaceSm(bool sync, int num)
{
	TCSelector s(num, 11);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("replaceSm");
	for (; s.check(); s++)
	{
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			//������� ��������� messageId �� sms ��������� ��������
			PduData* replacePduData = NULL;
			if (fixture->pduReg)
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu();
			}
			transmitter->setupRandomCorrectReplaceSmPdu(pdu, replacePduData);
			switch (s.value())
			{
				case 1: //������ ����������
					//__tc__("replaceSm");
					break;
				case 2: //�������� ��� ������ ���� ��������
					{
						__tc__("replaceSm.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						pdu->set_registredDelivery(0);
					}
					break;
				case 3: //���� ���������� ��� ����������
					{
						__tc__("replaceSm.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxDeliveryPeriod), t, time(NULL), __absoluteTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 4: //���� ���������� ������ �������������
					{
						__tc__("replaceSm.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 5: //waitTime > validTime
					{
						__tc__("replaceSm.waitTimeInvalid");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(maxWaitTime);
						time_t waitTime = validTime + rand1(maxWaitTime);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 6: //������ ���� ���������
					__tc__("replaceSm.smLengthMarginal");
					pdu->set_shortMessage(NULL, 0);
					//pdu->set_shortMessage("", 0);
					break;
				case 7: //���� ��������� ������������ �����
					{
						__tc__("replaceSm.smLengthMarginal");
						auto_ptr<char> msg = rand_char(MAX_SM_LENGTH);
						pdu->set_shortMessage(msg.get(), MAX_SM_LENGTH);
					}
					break;
				case 8: //����������� ��������� �� ����������
					{
						__tc__("replaceSm.msgIdNotExist");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
						replacePduData = NULL;
					}
					break;
				case 9: //��������� ��� ����� ����������� ���������
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.replaceReplacedEnrote");
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				case 10: //���������� ������������� ���������, �� ������������
					//��� � ��������� ���������.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = fixture->pduReg->getLastRemovedPdu();
						if (finalPduData)
						{
							__tc__("replaceSm.replaceFinal");
							pdu->set_messageId(finalPduData->smsId.c_str());
						}
					}
					break;
				case 11: //��������� ������������� ���������, �� ������������
					//��� � �������� ��������� ��������.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.replaceRepeatedDeliveryEnrote");
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			transmitter->sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

uint32_t SmppProtocolTestCases::sendDeliverySmRespOk(PduDeliverySm& pdu, bool sync)
{
	__trace__("sendDeliverySmRespOk()");
	__decl_tc__;
	try
	{
		__tc__("sendDeliverySmResp.sendOk");
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
		transmitter->sendDeliverySmResp(respPdu, sync);
		__tc_ok__;
		return ESME_ROK;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return 0xffffffff;
}

uint32_t SmppProtocolTestCases::sendDeliverySmRespRetry(PduDeliverySm& pdu,
	bool sync, int num)
{
	__trace__("sendDeliverySmRespRetry()");
	TCSelector s(num, 4);
	__decl_tc__;
	try
	{
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		switch (s.value())
		{
			case 1: //�� ���������� �������
				__tc__("sendDeliverySmResp.sendRetry.notSend");
				respPdu.get_header().set_commandStatus(0xffffffff);
				break;
			case 2: //��������� ������ �� ������� sme, ������ �� ��������� ��������
				__tc__("sendDeliverySmResp.sendRetry.tempAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_T_APPN);
				transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 3: //������������ ������� ������� sme
				__tc__("sendDeliverySmResp.sendRetry.msgQueueFull");
				respPdu.get_header().set_commandStatus(ESME_RMSGQFUL);
				transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 4: //��������� ������� � ������������ sequence_number
				__tc__("sendDeliverySmResp.sendRetry.invalidSequenceNumber");
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(0xffffffff);
				//respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
				transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			default:
				__unreachable__("Invalid num");
		}
		__tc_ok__;
		return respPdu.get_header().get_commandStatus();
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return 0xffffffff;
}

uint32_t SmppProtocolTestCases::sendDeliverySmRespError(PduDeliverySm& pdu,
	bool sync, int num)
{
	__trace__("sendDeliverySmRespError()");
	TCSelector s(num, 3);
	__decl_tc__;
	try
	{
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		switch (s.value())
		{
			case 1: //��������� ������� � ����� ������ 0x1-0x10f
				__tc__("sendDeliverySmResp.sendError.standardError");
				respPdu.get_header().set_commandStatus(rand1(0x10f));
				break;
			case 2: //��������� ������� � ����� ������:
				//0x110-0x3ff - Reserved for SMPP extension
				//0x400-0x4ff - Reserved for SMSC vendor specific
				__tc__("sendDeliverySmResp.sendError.reservedError");
				respPdu.get_header().set_commandStatus(rand2(0x110, 0x4ff));
				break;
			case 3: //��������� ������� � ����� ������ >0x500 - Reserved
				__tc__("sendDeliverySmResp.sendError.outRangeError");
				respPdu.get_header().set_commandStatus(rand2(0x500, INT_MAX));
				break;
			/*
			case 4: //������������ ������ �� ������� sme, ����� �� ���� ����������� ���������
				__tc__("sendDeliverySmResp.sendError.permanentAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_P_APPN);
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		//����������� �����, ����� ������������� �������� smpp ������
		if (respPdu.get_header().get_commandStatus() == ESME_RX_P_APPN)
		{
			return sendDeliverySmRespError(pdu, sync, num);
		}
		transmitter->sendDeliverySmResp(respPdu, sync);
		__tc_ok__;
		return respPdu.get_header().get_commandStatus();
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return 0xffffffff;
}

}
}
}

