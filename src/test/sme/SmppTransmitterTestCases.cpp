#include "SmppTransmitterTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using namespace smsc::sms; //constants
using namespace smsc::test; //config constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::sms; //constants
using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;

SmppTransmitterTestCases::SmppTransmitterTestCases(SmppSession* sess,
	const SmeSystemId& id, const Address& addr, const SmeRegistry* _smeReg,
	RouteChecker* _routeChecker, SmppPduChecker* _pduChecker)
	: session(sess), systemId(id), smeAlias(addr), smeReg(_smeReg),
	routeChecker(_routeChecker), pduChecker(_pduChecker)
{
	__require__(session);
	__require__(smeReg);
	__require__(routeChecker);
	__require__(pduChecker);
	pduReg = smeReg->getPduRegistry(smeAlias); //����� ���� NULL
}

Category& SmppTransmitterTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTransmitterTestCases");
	return log;
}

TCResult* SmppTransmitterTestCases::submitSmAssert(int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_SUBMIT_SM_ASSERT, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm pdu;
			switch (s.value())
			{
				case 1: //serviceType ������ ������������ �����
					{
						char serviceType[MAX_SERVICE_TYPE_LENGTH + 10];
						rand_char(MAX_SERVICE_TYPE_LENGTH + 1, serviceType);
						pdu.get_message().set_serviceType(serviceType);
					}
					break;
				case 2: //������ �����
					{
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						addr.set_value("");
						pdu.get_message().set_dest(addr);
					}
					break;
				case 3: //destinationAddress ������ ������������ �����
					{
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
						char msg[MAX_MSG_BODY_LENGTH + 10];
						rand_char(MAX_MSG_BODY_LENGTH + 1, msg);
						pdu.get_message().set_shortMessage(msg, MAX_MSG_BODY_LENGTH + 1);
					}
					break;
				default:
					throw s;
			}
			__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmAssert", systemId, &pdu);
			res->addFailure(s.value());
		}
		catch (...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

TCResult* SmppTransmitterTestCases::submitSmSync(int num)
{
	return submitSm(TC_SUBMIT_SM_SYNC, true, num);
}

TCResult* SmppTransmitterTestCases::submitSmAsync(int num)
{
	return submitSm(TC_SUBMIT_SM_ASYNC, false, num);
}

void SmppTransmitterTestCases::fillSubmitSmPduData(PduData* pduData,
	PduSubmitSm* pdu, PduData* replacePduData)
{
	//��� ������ ����� ������� ��������, ��������� �������� �
	//checkSubmitSmResp, checkSubmitTime, checkWaitTime, checkValidTime
	//pduData.responseFlag = PDU_REQUIRED_FLAG;
	pduData->deliveryFlag = routeChecker->isDestReachable(
		pdu->get_message().get_dest(), false) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	uint8_t regDelivery = pdu->get_message().get_registredDelivery();
	pduData->deliveryReceiptFlag =
		((regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == NO_SMSC_DELIVERY_RECEIPT ?
		PDU_NOT_EXPECTED_FLAG : PDU_REQUIRED_FLAG);
	pduData->intermediateNotificationFlag =
		((regDelivery & INTERMEDIATE_NOTIFICATION_REQUESTED) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG);
	pduData->replacePdu = replacePduData;
	if (replacePduData)
	{
		replacePduData->replacedByPdu = pduData;
	}
}

//����� ����� ���������� ������������� �� pduData->mutex
vector<int> SmppTransmitterTestCases::submitAndRegisterSmSync(PduSubmitSm* pdu,
	PduData* replacePduData)
{
	__require__(pduReg);
	//��������������� ����������� pdu
	PduData* pduData;
	{
		MutexGuard mguard(pduReg->getMutex());
		pduData = new PduData(pdu->get_optional().get_userMessageReference(),
			time(NULL),
			max(time(NULL), SmppUtil::string2time(pdu->get_message().get_scheduleDeliveryTime(), time(NULL))),
			SmppUtil::string2time(pdu->get_message().get_validityPeriod(), time(NULL)),
			reinterpret_cast<SmppHeader*>(pdu));
		pdu->get_header().set_sequenceNumber(0);
		fillSubmitSmPduData(pduData, pdu, replacePduData);
		pduReg->registerPdu(pduData);
	}
	//��������� pdu
	__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmSyncBefore", systemId, pdu);
	PduSubmitSmResp* respPdu = session->getSyncTransmitter()->submit(*pdu);
	time_t respTime = time(NULL);
	__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmSyncAfter", systemId, pdu);
	__dumpPdu__("SmppTransmitterTestCases::processSubmitSmRespSync", systemId, respPdu);
	//��������� ����������� � �������� pdu
	vector<int> res;
	{
		MutexGuard mguard(pduReg->getMutex());
		if (!respPdu)
		{
			pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
			res.push_back(100);
		}
		else
		{
			pduData->smsId = SmppUtil::convert(respPdu->get_messageId());
			pduData->responseFlag = PDU_REQUIRED_FLAG;
			res = pduChecker->checkSubmitSmResp(pduData, *respPdu, respTime);
			delete respPdu; //disposePdu
		}
		pduReg->updatePdu(pduData);
	}
	return res;
}

//����� ����� ���������� ������������� �� pduReg->getMutex()
vector<int> SmppTransmitterTestCases::submitAndRegisterSmAsync(PduSubmitSm* pdu,
	PduData* replacePduData)
{
	__require__(pduReg);
	//��������� pdu
	__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmAsyncBefore", systemId, pdu);
	MutexGuard mguard(pduReg->getMutex());
	PduSubmitSmResp* respPdu = session->getAsyncTransmitter()->submit(*pdu);
	__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmAsyncAfter", systemId, pdu);
	__dumpPdu__("SmppTransmitterTestCases::processSubmitSmRespAsync", systemId, respPdu);
	//����������� pdu
	PduData* pduData = new PduData(pdu->get_optional().get_userMessageReference(),
		time(NULL),
		max(time(NULL), SmppUtil::string2time(pdu->get_message().get_scheduleDeliveryTime(), time(NULL))),
		SmppUtil::string2time(pdu->get_message().get_validityPeriod(), time(NULL)),
		reinterpret_cast<SmppHeader*>(pdu));
	fillSubmitSmPduData(pduData, pdu, replacePduData);
    pduData->responseFlag = PDU_REQUIRED_FLAG;
	pduReg->registerPdu(pduData);
	vector<int> res;
	if (respPdu)
	{
		res.push_back(100);
		delete respPdu; //disposePdu
	}
	return res;
}

TCResult* SmppTransmitterTestCases::submitSm(const char* tc, bool sync, int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(tc, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			SmppUtil::setupRandomCorrectSubmitSmPdu(pdu);
			 //Default message Type (i.e. normal message)
			pdu->get_message().set_esmClass(
				pdu->get_message().get_esmClass() & 0xc3);
			//���� ��������� ������������� �������� ��� ������������� �����������,
			//���������� ���������� ��������
			uint8_t regDelivery = pdu->get_message().get_registredDelivery();
			if ((regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == FINAL_SMSC_DELIVERY_RECEIPT ||
				(regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == FAILURE_SMSC_DELIVERY_RECEIPT ||
				(regDelivery & INTERMEDIATE_NOTIFICATION_REQUESTED))
			{
				SmppTime t;
				time_t waitTime = time(NULL) + rand2(10, 60);
				time_t validTime = waitTime + rand1(60);
				pdu->get_message().set_scheduleDeliveryTime(
					SmppUtil::time2string(waitTime, t, time(NULL), __numTime__));
				pdu->get_message().set_validityPeriod(
					SmppUtil::time2string(validTime, t, time(NULL), __numTime__));
			}
			//src �����
			PduAddress addr;
			SmppUtil::convert(smeAlias, &addr);
			pdu->get_message().set_source(addr);
			//��������� dest ����� ��� �������� ������� ���������
			PduAddress destAddr;
			const Address* tmp = smeReg->getRandomAddress();
			__require__(tmp);
			SmppUtil::convert(*tmp, &destAddr);
			pdu->get_message().set_dest(destAddr);
			PduData* replacePduData = NULL;
			switch (s.value())
			{
				/*
				case 1: //������ serviceType
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					break;
				*/
				case 1: //serviceType ������������ �����
					{
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 2: //�������� ��� ������ ���� ��������
					{
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(60), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						pdu->get_message().set_registredDelivery(0);
					}
					break;
				case 3: //���� ���������� ��� ����������
					{
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(60), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 4: //���� ���������� ������ �������������
					{
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 5: //waitTime > validTime
					{
						SmppTime t;
						time_t validTime = time(NULL) + rand1(60);
						time_t waitTime = validTime + rand1(60);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 6: //������ ���� ���������
					pdu->get_message().set_shortMessage(NULL, 0);
					//pdu->get_message().set_shortMessage("", 0);
					break;
				case 7: //���� ��������� ������������ �����
					{
						ShortMessage msg;
						rand_char(MAX_MSG_BODY_LENGTH, msg);
						pdu->get_message().set_shortMessage(msg, MAX_MSG_BODY_LENGTH);
					}
					break;
				case 8: //msgRef ���������� (������������ msgRef �����������)
					//�������� GSM 03.40 ����� 9.2.3.25 ���� ���������
					//TP-MR, TP-DA, OA, �� ��� ETSI_REJECT_IF_PRESENT ����� ������.
					//��� SMPP ��� ������ �������� ���������� �� msgRef.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduRegistry::PduDataIterator* it =
							pduReg->getPduByWaitTime(time(NULL) + rand2(20, 30), LONG_MAX);
						PduData* pendingPduData = it->next();
						delete it;
						if (pendingPduData)
						{
							__require__(pendingPduData->pdu &&
								pendingPduData->pdu->get_commandId() == SUBMIT_SM);
							PduSubmitSm* pendingPdu =
								reinterpret_cast<PduSubmitSm*>(pendingPduData->pdu);
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								pendingPdu->get_message().get_dest());
							pdu->get_optional().set_userMessageReference(
								pendingPdu->get_optional().get_userMessageReference());
						}
					}
					break;
				case 9: //�������� �������������� ��������� ��� ��������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduRegistry::PduDataIterator* it =
							pduReg->getPduByWaitTime(time(NULL) + rand2(20, 30), LONG_MAX);
						PduData* pendingPduData = it->next();
						delete it;
						if (pendingPduData)
						{
							__require__(pendingPduData->pdu &&
								pendingPduData->pdu->get_commandId() == SUBMIT_SM);
							PduSubmitSm* pendingPdu =
								reinterpret_cast<PduSubmitSm*>(pendingPduData->pdu);
							pdu->get_message().set_serviceType(
								pendingPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								pendingPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(0);
						}
					}
					break;
				case 10: //�������� �������������� ��������� � ���������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduRegistry::PduDataIterator* it =
							pduReg->getPduByWaitTime(time(NULL) + rand2(20, 30), LONG_MAX);
						replacePduData = it->next();
						delete it;
						if (replacePduData)
						{
							__require__(replacePduData->pdu &&
								replacePduData->pdu->get_commandId() == SUBMIT_SM);
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							//���������� ��������� ����� ������������ ������ �������������
							SmppTime tmp;
							SmppUtil::time2string(replacePduData->waitTime - rand1(10),
								tmp, time(NULL), __numTime__);
							pdu->get_message().set_scheduleDeliveryTime(tmp);
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				default:
					throw s;
			}
			//��������� � ���������������� pdu
			if (pduReg)
			{
				pdu->get_optional().set_userMessageReference(pduReg->nextMsgRef());
				vector<int> checkRes;
				if (sync)
				{
					checkRes = submitAndRegisterSmSync(pdu, replacePduData);
					getLog().debug("[%d]\tsubmitSmSync(%d): sequenceNumber = %d",
						thr_self(), s.value(), pdu->get_header().get_sequenceNumber());
				}
				else
				{
					checkRes = submitAndRegisterSmAsync(pdu, replacePduData);
					getLog().debug("[%d]\tsubmitSmAsync(%d): sequenceNumber = %d",
						thr_self(), s.value(), pdu->get_header().get_sequenceNumber());
				}
				for (int i = 0; i < checkRes.size(); i++)
				{
					res->addFailure(checkRes[i] > 0 ? 100 + checkRes[i] : checkRes[i]);
				}
				//pdu life time ������������ PduRegistry
				//disposePdu(pdu);
			}
			else
			{
				if (sync)
				{
					PduSubmitSmResp* respPdu =
						session->getSyncTransmitter()->submit(*pdu);
					if (respPdu)
					{
						delete respPdu; //disposePdu
					}
				}
				else
				{
					session->getAsyncTransmitter()->submit(*pdu);
				}
				delete pdu; //disposePdu
			}
		}
		catch(...)
		{
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

}
}
}

