#include "SmppTransmitterTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using namespace smsc::sms; //constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::sms; //constants
using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;

SmppTransmitterTestCases::SmppTransmitterTestCases(SmppSession* sess,
	const Address& addr, const SmeRegistry* _smeReg,
	SmppPduChecker* _pduChecker)
	: session(sess), smeAddr(addr), smeReg(_smeReg),
	pduChecker(_pduChecker)
{
	__require__(session);
	__require__(smeReg);
	__require__(pduChecker);
	pduReg = smeReg->getPduRegistry(smeAddr); //����� ���� NULL
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
			__dumpPdu__("SmppTransmitterTestCases::submitSmAssert", pdu);
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

//����� ����� ���������� ������������� �� pduReg->getMutex()
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
			pdu->get_message().set_esmClass(
				pdu->get_message().get_esmClass() & 0xc3); //Default message Type (i.e. normal message)
			PduAddress addr;
			SmppUtil::convert(smeAddr, &addr);
			pdu->get_message().set_source(addr);
			//��������� dest ����� ��� ��������� � �������� ������� ���������
			PduAddress destAddr;
			const Address* tmp = smeReg->getRandomAddress();
			__require__(tmp);
			SmppUtil::convert(*tmp, &destAddr);
			pdu->get_message().set_dest(destAddr);
			if (pduReg)
			{
				pduReg->getMutex().Lock();
				pdu->get_optional().set_userMessageReference(pduReg->nextMsgRef());
			}
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
						SmppUtil::time2string(time(NULL) - rand1(60), t, __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
					}
					break;
				case 3: //���� ���������� ��� ����������
					{
						SmppTime t;
						SmppUtil::time2string(time(NULL) - rand1(60), t, __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 4: //���� ���������� ������ �������������
					{
						SmppTime t;
						SmppUtil::time2string(__maxValidPeriod__ + 1, t, __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 5: //waitTime > validTime
					{
						SmppTime t;
						time_t validTime = time(NULL) + rand1(60);
						time_t waitTime = validTime + rand1(60);
						SmppUtil::time2string(waitTime, t, __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, __numTime__);
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
							time_t waitTime = SmppUtil::string2time(
								replacePdu->get_message().get_scheduleDeliveryTime());
							SmppUtil::time2string(waitTime - rand1(10), tmp, __numTime__);
							pdu->get_message().set_scheduleDeliveryTime(tmp);
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				default:
					throw s;
			}
			//��������� � ���������������� pdu
			PduSubmitSmResp* respPdu = NULL;
			if (sync)
			{
				__dumpPdu2__("SmppTransmitterTestCases::submitSmSyncBefore", pdu);
				respPdu = session->getSyncTransmitter()->submit(*pdu);
				__dumpPdu2__("SmppTransmitterTestCases::submitSmSyncAfter", pdu);
				__dumpPdu2__("SmppTransmitterTestCases::processSubmitSmRespSync", respPdu);
				getLog().debug("[%d]\tsubmitSmSync(%d): seqNum = %d",
					thr_self(), s.value(), pdu->get_header().get_sequenceNumber());
				if (!respPdu)
				{
					res->addFailure(101);
				}
			}
			else
			{
				__dumpPdu2__("SmppTransmitterTestCases::submitSmAsyncBefore", pdu);
				respPdu = session->getAsyncTransmitter()->submit(*pdu);
				__dumpPdu2__("SmppTransmitterTestCases::submitSmAsyncAfter", pdu);
				__dumpPdu2__("SmppTransmitterTestCases::processSubmitSmRespAsync", respPdu);
				getLog().debug("[%d]\tsubmitSmAsync(%d): seqNum = %d",
					thr_self(), s.value(), pdu->get_header().get_sequenceNumber());
				if (respPdu)
				{
					res->addFailure(102);
				}
			}
			if (pduReg)
			{
				//���������������� pdu
				PduData pduData(reinterpret_cast<SmppHeader*>(pdu));
				if (respPdu)
				{
					pduData.smsId = SmppUtil::convert(respPdu->get_messageId());
					pduData.responseFlag = true;
				}
				pduData.msgRef = pdu->get_optional().get_userMessageReference();
				pduData.submitTime = time(NULL);
				pduData.waitTime =
					SmppUtil::string2time(pdu->get_message().get_scheduleDeliveryTime());
				pduData.validTime =
					SmppUtil::string2time(pdu->get_message().get_validityPeriod());
				//pduData.responseFlag = false;
				pduData.deliveryFlag = false;
				//���� delivery receipt � intermediate notifications �� ������ ����,
				//�������� �� ��� ��� ���������� 
				pduData.deliveryReceiptFlag =
					pdu->get_message().get_registredDelivery() &
					SMSC_DELIVERY_RECEIPT_BITS == NO_SMSC_DELIVERY_RECEIPT;
				pduData.intermediateNotificationFlag =
					pdu->get_message().get_registredDelivery() &
					INTERMEDIATE_NOTIFICATION_REQUESTED;
				pduData.replacePdu = replacePduData;
				pduReg->putPdu(pduData);
				//��������� �������
				if (respPdu)
				{
					vector<int> tmp =
						pduChecker->checkSubmitSmResp(&pduData, *respPdu);
					for (int i = 0; i < tmp.size(); i++)
					{
						res->addFailure(tmp[i] > 0 ? 110 + tmp[i] : tmp[i]);
					}
				}
				pduReg->getMutex().Unlock();
				//pdu life time ������������ PduRegistry
				//disposePdu(pdu);
			}
			else
			{
				//���� ������ ��������� �� ���� (��� performance testing)
				delete pdu; //disposePdu
			}
			delete respPdu; //disposePdu
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

