#include "SmeTestCases.hpp"
#include "util/Exception.hpp"

namespace smsc {
namespace test {
namespace sme {

using namespace smsc::smpp::SmppCommandSet; //constants
using smsc::util::Logger;
using smsc::util::Exception;

SmeTestCases::SmeTestCases(const char* _systemId, const Address& _origAddr,
	const char* _serviceType)
	: sme(NULL), bindType(0), systemId(_systemId), origAddr(_origAddr),
	  serviceType(_serviceType);
{
	sme = new BaseSme("localhost", 1111, systemId.c_str());
	if (!sme->init())
	{
		throw Exception("Initialization failed");
	}
}

SmeTestCases::~SmeTestCases()
{
	if (sme)
	{
		delete sme;
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
					sme->bindsme(); //��� ����������� ������ ���
					bindType = BIND_TRANCIEVER;
					break;
				default:
					throw s;
			}
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
					sme->bindsme(); //��� ����������� ������ ���
					bindType = BIND_TRANCIEVER;
					break;
				default:
					throw s;
			}
		}
		catch(...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

TCResult* SmeTestCases::submitCorrectSms(const RouteRegistry& routeReg,
	MessageRegistry& msgReg, int num)
{
	int num1 = 18; int num2 = 3;
	TCSelector s(num, num1 * num2);
	TCResult* res = new TCResult(TC_SUBMIT_CORRECT_SMS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			SMS sms = new SMS();
			SmsUtil::setupRandomCorrectSms(sms);
			sms->setOriginatingAddress(origAddr);
			//no interworking, but SME-to-SME protocol
			//������ �������� ��������� ������������������
			sms->setEServiceType(serviceType.c_str());
			switch(s.value1(num1))
			{
				case 1: //������ ����������
					break;
				case 2: //�������� ��� ������ ���� ��������
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
			//������� destAddr
			const Address* destAddr;
			bool reachableDestAddr = false;
			switch (s.value2(num1))
			{
				case 1:
					destAddr = routeReg.getRandomReachableDestAddress(origAddr);
					reachableDestAddr = true;
					break;
				case 2:
					destAddr = routeReg.getRandomNonReachableDestAddress(origAddr);
					break;
				case 3:
					{
						Address* addr = new Address();
						SmsUtil::setupRandomCorrectAddress(addr);
						destAddr = addr;
					}
					break;
				default:
					throw s;
			}
			//��������� sms
			if (destAddr)
			{
				MutexGuard mguard(msgReg.getMutex(origAddr));
				sms->setMessageReference(msgReg.nextMsgRef(origAddr));
				sms->setDestinationAddress(destAddr);
				if (sme->sendSms(sms))
				{
					//���������������� sms
					MsgData msgData(const SMSId = ???, sms->getMessageReference(),
						sms->getWaitTime(), sms, NULL);
					msgData.deliveryReceiptFlag = (sms->getDeliveryReport() &
						SMSC_DELIVERY_RECEIPT_BITS == NO_SMSC_DELIVERY_RECEIPT);
					msgData.intermediateNotificationFlag = (sms->getDeliveryReport() &
						INTERMEDIATE_NOTIFICATION_REQUESTED);
					msgData.deliveryFlag = !reachableDestAddr;
					if (!msgData.complete())
					{
						msgReg.putMsg(origAddr, msgData);
					}
				}
				else
				{
					res->addFailure(101);
				}
				delete destAddr;
			}
			else
			{
				res->addFailure(102);
			}
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

void SmeTestCases::processNormalSms(const SMS& sms, MessageRegistry& msgReg,
	TCResult* res)
{
	//�������� ������������ sms
	MutexGuard mguard(msgReg.getMutex(sms.getOriginatingAddress()));
	MsgData* data = msgReg.getMsg(sms.getOriginatingAddress(),
		sms.getMessageReference());
	if (!data || !data->sms)
	{
		res->addFailure(100);
		return;
	}
	//�������� ���������� ���������
	SmsCompareFlag flag = IGNORE_STATE | IGNORE_ORIGINATING_DESCRIPTOR |
		IGNORE_DESTINATION_DESCRIPTOR | IGNORE_LAST_TIME | IGNORE_NEXT_TIME |
		IGNORE_ARCHIVATION_REQUESTED | IGNORE_ATTEMPTS_COUNT;
	vector<int> tmp = SmsUtil::compareMessages(sms, correctSms, flag);
	for (int i = 0; i < tmp.size(); i++)
	{
		res->addFailure(100 + tmp[i]);
	}
	data->deliveryFlag = true;
	//���� ��� ��������, ������� sms
	if (data->complete())
	{
		msgReg.removeMsg(sms.getOriginatingAddress(), data->smsId);
	}
}

void SmeTestCases::processDeliveryReceiptSms(const SMS& sms,
	MessageRegistry& msgReg, TCResult* res)
{
	//�������� ������������ sms
	MutexGuard mguard(msgReg.getMutex(sms.getDestinationAddress()));
	MsgData* data = msgReg.getMsg(sms.getDestinationAddress(),
		sms.getRecieptSmsId());
	if (!data || !data->sms)
	{
		res->addFailure(200);
		return;
	}
	switch (data->sms->getDeliveryReport() & SMSC_DELIVERY_RECEIPT_BITS)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
			res->addFailure(201);
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			//������ ���� ������� ������
			if (!sms.getFailureCause())
			{
				res->addFailure(202);
			}
			//break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
			//������ ������ ������ ���� ������������� ��������
			if (data->deliveryReceiptFlag)
			{
				res->addFailure(203);
			}
			//������ ������ ���� ���������
			if (sms.getState() == ENROUTE)
			{
				res->addFailure(204);
			}
			else
			{
				data->deliveryReceiptFlag = true;
			}
			break;
		default:
			//������������ ��������
			res->addFailure(205);
	}
	//��������� ���������� sms
	???
	//���� ��� ��������, ������� sms
	if (data->complete())
	{
		msgReg.removeMsg(sms.getDestinationAddress(), data->smsId);
	}
}

void SmeTestCases::processIntermediateNotificationSms(const SMS& sms,
	MessageRegistry& msgReg, TCResult* res)
{
	//�������� ������������ sms
	MutexGuard mguard(msgReg.getMutex(sms.getDestinationAddress()));
	MsgData* data = msgReg.getMsg(sms.getDestinationAddress(),
		sms.getRecieptSmsId());
	if (!data || !data->sms)
	{
		res->addFailure(300);
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
		res->addFailure(301);
	}
	//��������� ���������� sms
	???
	//���� ��� ��������, ������� sms
	if (data->complete())
	{
		msgReg.removeMsg(sms.getDestinationAddress(), data->smsId);
	}
}

TCResult* SmeTestCases::processSms(const SMS& sms, const RouteRegistry& routeReg,
	MessageRegistry& msgReg)
{
	TCResult* res = new TCResult(TC_PROCESS_SMS);
	//�������� ������������ ��������
	const SmeSystemId& correctId = *routeReg.lookup(sms.getOriginatingAddress(),
		sms.getDestinationAddress());
	if (systemId != correctId)
	{
		res->addFailure(1);
	}
	//���������� ��� ���������
	switch (sms.getEsmClass() & ESM_CLASS_MESSAGE_TYPE_BITS)
	{
		case ESM_CLASS_NORMAL_MESSAGE:
			//���� ������ - 1xx
			processNormalSms(sms, msgReg, res);
			break;
		case ESM_CLASS_DELIVERY_RECEIPT:
			//���� ������ - 2xx
			processDeliveryReceiptSms(sms, msgReg, res);
			break;
		case ESM_CLASS_INTERMEDIATE_NOTIFICATION:
			//���� ������ - 3xx
			processIntermediateNotificationSms(sms, msgReg, res);
			break;
		default:
			res->addFailure(400);
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

