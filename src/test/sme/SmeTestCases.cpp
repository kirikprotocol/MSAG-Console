#include "SmeTestCases.hpp"
#include "util/debug.h"
#include "util/Exception.hpp"

namespace smsc {
namespace test {
namespace sme {

using namespace smsc::smpp::SmppCommandSet; //constants
using smsc::util::Logger;
using smsc::util::Exception;

SmeTestCases::SmeTestCases(const SmeConfig& config, const Address& addr,
	const EService type, const SmeRegistry* _smeReg,
	const RouteRegistry* _routeReg, ResultHandler* handler)
	: session(NULL), origAddr(addr), serviceType(type), smeReg(_smeReg),
	routeReg(_routeReg), resultHandler(handler)
{
	__require__(routeReg);
	__require__(resultHandler);
	session = new SmppSession(config, this);
	if (smeReg)
	{
		pduReg = smeReg->getPduRegistry(origAddr);
	}
}

SmeTestCases::~SmeTestCases()
{
	if (session)
	{
		try
		{
			session.close();
		}
		catch(...)
		{
			//nothing
		}
		delete session;
	}
}

Category& SmeTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeTestCases");
	return log;
}

TCResult* SmeTestCases::bindRegisteredSme(int num)
{
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_REGISTERED_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
				case 2: //BIND_TRANSMITTER
				case 3: //BIND_TRANCIEVER
					break;
				default:
					throw s;
			}
			session->connect();
		}
		catch(...)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* SmeTestCases::bindNonRegisteredSme(int num)
{
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_NON_REGISTERED_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
				case 2: //BIND_TRANSMITTER
				case 3: //BIND_TRANCIEVER
					break;
				default:
					throw s;
			}
			session->connect();
			res->addFailure(101);
		}
		catch(...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

bool SmeTestCases::setupRandomDestAddr(PduAddress* destAddr, int num)
{
	switch (num)
	{
		case 1: //�������� ������������������� � ���������� sme
			{
				const Address* addr =
					routeReg->getRandomReachableDestAddress(origAddr);
				SmppUtil::convert(addr ? *addr : origAddr, destAddr);
				return true;
			}
		case 2: //�������� �������������������, �� ������������ sme
			{
				const Address* addr =
					routeReg->getRandomNonReachableDestAddress(origAddr);
				SmppUtil::convert(addr ? *addr : origAddr, destAddr);
				return false;
			}
		case 3: //�������� ��������������������� sme
			{
				Address addr;
				SmsUtil::setupRandomCorrectAddress(addr);
				SmppUtil::convert(addr, destAddr);
				return false;
			}
		default:
			throw "";
	}
}

void SmeTestCases::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu, int num)
{
	SmppUtil::setupRandomCorrectSubmitSmPdu(pdu);
	pdu->get_message().set_serviceType(config.serviceType);
	pdu->get_message().set_source(config.addr);
	//pdu->get_message().set_dest(addr);

	//no interworking, but SME-to-SME protocol
	//������ �������� ��������� ������������������
	/*
	pdu->get_message().set_validityPeriod(time2string(time(NULL) + 10, tmp, rand1(2)));
	pdu->get_message().set_registredDelivery(rand0(255));
	pdu->get_message().set_replaceIfPresentFlag(0); //Don�t replace
	ShortMessage msg;
	int len = setupRandomCorrectShortMessage(&msg);
	pdu->get_message().set_dataCoding(rand0(255));
	pdu->get_message().set_smDefaultMsgId(0); //��� ��� ��� �����
	pdu->get_message().set_shortMessage(msg, len);
	*/

	/*
	switch(s.value1(num1))
	{
		case 1: //������ ����������
			break;
		case 2: //�������� ��� ������ ���� ��������
			pdu->get_message().set_scheduleDeliveryTime(
				time2string(time(NULL) - 10, tmp, rand1(2)));
			sms->setWaitTime(time(NULL) - 10);
			break;
		case 3: //���������� ��������
			sms->setWaitTime(time(NULL) + 10);
			break;
		case 4: //�������� ������������ ���������
			sms->setValidTime(time(NULL) - 10);
			break;
		case 5: //��������� ������� �� ����� ���������� ��������
			sms->setWaitTime(time(NULL) + 20);
			sms->setValidTime(time(NULL) + 10);
			break;
		case 6: //������� submitTime ������������ ����� SMSC
			sms->setSubmitTime(time(NULL) + rand2(-3600, 3600));
			break;
		case 7: //No SMSC Delivery Receipt requested
			sms->setDeliveryReport(NO_SMSC_DELIVERY_RECEIPT);
			break;
		case 8:
			//SMSC Delivery Receipt requested where final delivery
			//outcome is delivery success or failure
			sms->setDeliveryReport(FINAL_SMSC_DELIVERY_RECEIPT);
			break;
		case 9:
			//SMSC Delivery Receipt requested where the final
			//delivery outcome is delivery failure
			sms->setDeliveryReport(FAILURE_SMSC_DELIVERY_RECEIPT);
			break;
		case 10:
			//Intermediate notification requested
			sms->setDeliveryReport(INTERMEDIATE_NOTIFICATION_REQUESTED);
			break;
		case 11:
			//��������� ��������� ������������ ������������� ���������
			sms->setArchivationRequested(rand0(1));
			break;
		case 12: //������ ���� ���������
			sms.setMessageBody(0, 10, false, NULL);
			break;
		case 13: //������ ���� ���������
			{
				uint8_t body[] = {0};
				sms.setMessageBody(0, 20, false, tmp);
			}
			break;
		case 14: //���� ��������� ������������ �����
			{
				auto_ptr<uint8_t> body = rand_uint8_t(MAX_MSG_BODY_LENGTH);
				sms.setMessageBody(MAX_MSG_BODY_LENGTH, 30, false, body.get());
			}
			break;
		case 15:
			//Default SMSC Mode (e.g. Store and Forward)
			sms.setEsmClass(ESM_CLASS_DEFAULT_SMSC_MODE);
			break;
		case 16:
			//Datagram mode
			sms.setEsmClass(ESM_CLASS_DATAGRAM_MODE);
			break;
		case 17:
			//Forward (i.e. Transaction) mode
			sms.setEsmClass(ESM_CLASS_FORWARD_MODE);
			break;
		case 18:
			//Store and Forward mode
			sms.setEsmClass(ESM_CLASS_STORE_AND_FORWARD_MODE);
			break;
		default:
			throw s;
	}
	*/
}

const PduSubmitSmResp* SmeTestCases::submitRegisterPdu(
	SmppTransmitter* transmitter, PduSubmitSm* pdu, bool reachableDestAddr)
{
	if (!smeReg)
	{
		PduSubmitSmResp* respPdu = transmitter()->submit(pdu);
		delete pdu;
		return respPdu;
	}
	//�������� pduReg, ����� ������� ��� ����������� �� ������ ������,
	//��� ������� ����� ���������
	MutexGuard mguard(pduReg->getMutex());
	uint16_t msgRef = pduReg->nextMsgRef();
	pdu->get_optional().set_userMessageReference(msgRef);
	//��������� pdu
	PduSubmitSmResp* respPdu = transmitter()->submit(pdu);
	//���������������� pdu
	PduData pduData(msgRef, pdu->get_message().get_scheduleDeliveryTime(), pdu);
	pduData.deliveryReceiptFlag = pdu->get_message().get_registredDelivery() &
		SMSC_DELIVERY_RECEIPT_BITS == NO_SMSC_DELIVERY_RECEIPT;
	pduData->intermediateNotificationFlag =
		pdu->get_message().get_registredDelivery() &
		INTERMEDIATE_NOTIFICATION_REQUESTED;
	pduData->deliveryFlag = !reachableDestAddr;
	pduReg->putPdu(pduData);
	//pdu life time ������������ PduRegistry
	//delete pdu;
	return respPdu;
}

TCResult* SmeTestCases::submitSmSync(int num)
{
	int num1 = ; int num2 = 3;
	TCSelector s(num, num1 * num2);
	TCResult* res = new TCResult(TC_SUBMIT_SM_SYNC, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			setupRandomCorrectSubmitSmPdu(pdu, s.value1(num1));
			bool reachableDestAddr = setupRandomDestAddr(
				pdu->get_message().get_dest(), s.value2(num1));
			//��������� � ���������������� sms
			const PduSubmitSmResp* respPdu = submitRegisterPdu(
				session->getSyncTransmitter(), pdu, reachableDestAddr);
			if (!respPdu)
			{
				res->addFailure(101);
			}
			//���������� sms
			processSubmitSmResp(*respPdu);
			//delete pdu;
			delete respPdu;
		}
		catch(...)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* SmeTestCases::submitSmAsync(const RouteRegistry& routeReg,
	MessageRegistry& msgReg, int num)
{
	int num1 = ; int num2 = 3;
	TCSelector s(num, num1 * num2);
	TCResult* res = new TCResult(TC_SUBMIT_SMS_ASYNC, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			setupRandomCorrectSubmitSmPdu(pdu, s.value1(num1));
			bool reachableDestAddr = setupRandomDestAddr(
				pdu->get_message().get_dest(), s.value2(num1));
			//��������� � ���������������� sms
			const PduSubmitSmResp* respPdu = submitRegisterPdu(
				session->getAsyncTransmitter(), pdu, reachableDestAddr);
			if (respPdu)
			{
				res->addFailure(101);
			}
			//delete pdu;
		}
		catch(...)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

bool SmeTestCases::checkRemovePdu(const PduData* pduData)
{
	if (pduData->complete())
	{
		return pduReg->removePdu(*pduData);
	}
	return true;
}

void SmeTestCases::processSubmitSmResp(const PduSubmitSmResp &pdu)
{
	TCResult* res = new TCResult(TC_PROCESS_SUBMIT_SM_RESP);
	try
	{
		if (pduReg)
		{
			//�������� ������������ pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(pdu.get_header().get_sequenceNumber());
			//��� sequence number �� �������� ��� ���������������� pdu
			if (!pduData)
			{
				res->addFailure(1);
			}
			else
			{
				//������� ��� ��� �������
				if (pduData->responseFlag)
				{
					res->addFailure(2);
				}
				pduData->smsId = SmppUtil::convert(pdu.get_messageId());
				pduData->responseFlag = true;
				pduReg->putPdu(pduData); //�������� ������� ������ �� SMSId
				if (!checkRemovePdu(pduData))
				{
					res->addFailure(3);
				}
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	resultHandler->process(res);
}

void SmeTestCases::processDeliverySm(const PduDeliverySm &pdu)
{
	TCResult* res = new TCResult(TC_PROCESS_DELIVERY_SM);
	try
	{
		if (pduReg)
		{
			MutexGuard mguard(pduReg->getMutex());
			//� ���������� pdu ��� user_message_reference
			if (!pdu->get_optional().has_userMessageReference())
			{
				res->addFailure(1);
			}
			else
			{
				//�������� ������������ pdu
				PduData* pduData = pduReg->getPdu(
					pdu->get_optional().get_userMessageReference());
				//��� user_message_reference �� ���������� pdu
				//��� ���������������� ������������� pdu
				if (!pduData)
				{
					res->addFailure(2);
				}
				else
				{
					//������ pdu ��� ���� ���������� ���
					//������ �� ������ ���� ������������
					if (pduData->deliveryFlag)
					{
						res->addFailure(3);
					}
					//������������ ����� ��������
					if (abs(pduData->waitTime - time(NULL)) > 5)
					{
						res->addFailure(4);
					}
					//�������� ���������� ���������� � ������������ pdu
					vector<int> tmp = SmppUtil::comparePdu(pdu, pduData->pdu);
					for (int i = 0; i < tmp.size(); i++)
					{
						res->addFailure(100 + tmp[i]);
					}
					//��������� ���������
					pduData->deliveryFlag = true;
					if (!checkRemovePdu(pduData))
					{
						res->addFailure(5);
					}
				}
				/*
				SMSId smsId;
				const uint16_t msgRef;
				const time_t waitTime;
				const SmppHeader* pdu;
				bool responseFlag; //���� ��������� ��������
				bool deliveryFlag; //���� ��������� ��������� ����������
				bool deliveryReceiptFlag; //���� ��������� ������������� ��������
				bool intermediateNotificationFlag; //���� ��������� ���� �����������
				*/

			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	resultHandler->process(res);
}









TCResult* SmeTestCases::processNormalSms(const SMS& sms, MessageRegistry& msgReg)
{
	TCResult* res = new TCResult(TC_PROCESS_NORMAL_SMS);
	data->deliveryFlag = true;
	//���� ��� ��������, ������� sms
	if (data->complete())
	{
		msgReg.removeMsg(sms.getOriginatingAddress(), data->smsId);
	}
	return res;
}

TCResult* SmeTestCases::processDeliveryReceiptSms(const SMS& sms,
	messageRegistry& msgReg)
{
	TCResult* res = new TCResult(TC_PROCESS_DELIVERY_RECEIPT_SMS);
	//�������� ������������ sms
	MutexGuard mguard(msgReg.getMutex(sms.getDestinationAddress()));
	MsgData* data = msgReg.getMsg(sms.getDestinationAddress(),
		sms.getRecieptSmsId());
	if (!data || !data->sms)
	{
		res->addFailure(100);
		return;
	}
	switch (data->sms->getDeliveryReport() & SMSC_DELIVERY_RECEIPT_BITS)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
			res->addFailure(101);
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			//������ ���� ������� ������
			if (!sms.getFailureCause())
			{
				res->addFailure(102);
			}
			//break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
			//������ ������ ������ ���� ������������� ��������
			if (data->deliveryReceiptFlag)
			{
				res->addFailure(103);
			}
			//������ ������ ���� ���������
			if (sms.getState() == ENROUTE)
			{
				res->addFailure(104);
			}
			else
			{
				data->deliveryReceiptFlag = true;
			}
			break;
		default:
			//������������ ��������
			res->addFailure(105);
	}
	//��������� ���������� sms
	???
	//���� ��� ��������, ������� sms
	if (data->complete())
	{
		msgReg.removeMsg(sms.getDestinationAddress(), data->smsId);
	}
	return res;
}

TCResult* SmeTestCases::processIntermediateNotificationSms(const SMS& sms,
	MessageRegistry& msgReg)
{
	TCResult* res = new TCResult(TC_PROCESS_INTERMEDIATE_NOTIFICATION_SMS);
	//�������� ������������ sms
	MutexGuard mguard(msgReg.getMutex(sms.getDestinationAddress()));
	MsgData* data = msgReg.getMsg(sms.getDestinationAddress(),
		sms.getRecieptSmsId());
	if (!data || !data->sms)
	{
		res->addFailure(100);
		return;
	}
	if (data->sms->getDeliveryReport() & INTERMEDIATE_NOTIFICATION_REQUESTED)
	{
		//�������� ��������� �����������
		if (sms.getState() != ENROTE)
		{
			data->intermediateNotificationFlag = true;
		}
	}
	else
	{
		res->addFailure(101);
	}
	//��������� ���������� sms
	???
	//���� ��� ��������, ������� sms
	if (data->complete())
	{
		msgReg.removeMsg(sms.getDestinationAddress(), data->smsId);
	}
	return res;
}

TCResult* SmeTestCases::processSms(const SMS& sms, const RouteRegistry& routeReg,
	MessageRegistry& msgReg)
{
	TCResult* res;
	//���������� ��� ���������
	switch (sms.getEsmClass() & ESM_CLASS_MESSAGE_TYPE_BITS)
	{
		case ESM_CLASS_NORMAL_MESSAGE:
			res = processNormalSms(sms, msgReg, res);
			break;
		case ESM_CLASS_DELIVERY_RECEIPT:
			res = processDeliveryReceiptSms(sms, msgReg, res);
			break;
		case ESM_CLASS_INTERMEDIATE_NOTIFICATION:
			res = processIntermediateNotificationSms(sms, msgReg, res);
			break;
		default:
			res = new TCResult(TC_PROCESS_SMS);
			res->addFailure(201);
	}
	//�������� ������������ ��������
	const SmeSystemId* correctId = routeReg.lookup(sms.getOriginatingAddress(),
		sms.getDestinationAddress());
	if (!correctId)
	{
		res->addFailure(202);
	}
	else if (systemId != *correctId)
	{
		res->addFailure(203);
	}
	debug(res);
	return res;
}
	
TCResult* SmeTestCases::unbindBounded()
{
	TCResult* res = new TCResult(TC_UNBIND_BOUNDED);
	res.addFailure(100);
	debug(res);
	return res;
}

TCResult* SmeTestCases::unbindNonBounded()
{
	TCResult* res = new TCResult(TC_UNBIND_NON_BOUNDED);
	res.addFailure(100);
	debug(res);
	return res;
}

}
}
}

