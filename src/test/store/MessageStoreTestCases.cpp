#include "util/config/Manager.h"
#include "util/debug.h"
#include "MessageStoreTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "store/StoreManager.h"
#include "store/StoreExceptions.h"
#include <cstdlib>
#include <ctime>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::test::util; //TCSelector, TCResult
using namespace smsc::store; //StoreManager, CreateMode
using namespace smsc::sms; //constants, SMSId, SMS, AddressValue, EService, ...
using log4cpp::Category;
using smsc::util::Logger;
using smsc::test::sms::SmsUtil;
using smsc::util::AssertException;

Category& MessageStoreTestCases::log = Logger::getCategory("smsc.test.store.TestCases");

MessageStoreTestCases::MessageStoreTestCases()
{
	srand(time(NULL));
	msgStore = StoreManager::getMessageStore();
}

TCResult* MessageStoreTestCases::storeCorrectSms(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 13);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SMS, s.getChoice());
	for (; s.check(); s++)
	{
		SMS sms;
		try
		{
			SmsUtil::setupRandomCorrectSms(&sms);
			switch(s.value())
			{
				case 1: //������ ����������
					break;
				case 2: //originatingAddress ����������� �����
					//�������� SMPP v3.4 ������ ���� 0, �� �� ������ ���
					sms.setOriginatingAddress(1, 10, 20, "*");
					break;
				case 3: //originatingAddress ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setOriginatingAddress(MAX_ADDRESS_LENGTH, 20, 30,
							addr.get());
					}
					break;
				case 4: //destinationAddress ����������� �����
					sms.setDestinationAddress(1, 30, 40, "*");
					break;
				case 5: //destinationAddress ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setDestinationAddress(MAX_ADDRESS_LENGTH, 40, 50, 
							addr.get());
					}
					break;
				case 6: //������ imsi (��. GSM 09.02 ����� 12.2) � ������ msc (???)
					sms.setOriginatingDescriptor(0, NULL, 0, NULL, 10);
					break;
				case 7: //������ imsi (��. GSM 09.02 ����� 12.2) � ������ msc (???)
					sms.setOriginatingDescriptor(0, "*", 0, "*", 20);
					break;
				case 8: //imsi � msc ������ ������������ �����
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setOriginatingDescriptor(
							MAX_ADDRESS_LENGTH, mscAddr.get(),
							MAX_ADDRESS_LENGTH, imsiAddr.get(), 30);
					}
					break;
				case 9: //������ ���� ���������
					sms.setMessageBody(0, 10, false, NULL);
					break;
				case 10: //������ ���� ���������
					{
						uint8_t tmp[] = {0};
						sms.setMessageBody(0, 20, false, tmp);
					}
					break;
				case 11: //���� ��������� ������������ �����
					{
						auto_ptr<uint8_t> body = rand_uint8_t(MAX_MSG_BODY_LENGTH);
						sms.setMessageBody(MAX_MSG_BODY_LENGTH, 30, false, body.get());
					}
					break;
				case 12: //������� serviceType, NULL �����������
                   	sms.setEServiceType("");
					break;
				case 13: //serviceType ������������ �����
					{
						auto_ptr<char> type = rand_char(MAX_SERVICE_TYPE_LENGTH);
						sms.setEServiceType(type.get());
					}
					break;
				default:
					throw s;
			}
			SMSId smsId = msgStore->createSms(sms, CREATE_NEW);
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			*idp = 0;
			SmsUtil::clearSms(smsp);
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeCorrectSms(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSms, int num)
{
	TCSelector s(num, 7, 1000);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SMS, s.getChoice());
	//messageReference
	uint8_t msgRef = existentSms.getMessageReference();
	//originatingAddress
	const Address& origAddr = existentSms.getOriginatingAddress();
	AddressValue origAddrValue;
	uint8_t origAddrLength = origAddr.getValue(origAddrValue);
	uint8_t origAddrType = origAddr.getTypeOfNumber();
	uint8_t origAddrPlan = origAddr.getNumberingPlan();
	//destinationAddress
	const Address& destAddr = existentSms.getDestinationAddress();
	AddressValue destAddrValue;
	uint8_t destAddrLength = destAddr.getValue(destAddrValue);
	uint8_t destAddrType = destAddr.getTypeOfNumber();
	uint8_t destAddrPlan = destAddr.getNumberingPlan();
	//����� �����
	int randAddrLength = rand1(MAX_ADDRESS_VALUE_LENGTH);
	auto_ptr<char> randAddrValue = rand_char(randAddrLength);
	for (; s.check(); s++)
	{
		SMS sms;
		try
		{
			SmsUtil::setupRandomCorrectSms(&sms);
			SMSId smsId;
			CreateMode flag = ETSI_REJECT_IF_PRESENT;
			uint8_t shift = rand1(255);
			switch(s.value())
			{
				case 1001: //������� ������ � msgRef
					sms.setMessageReference(msgRef + shift);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddr);
					break;
				case 1002: //������� ������ � origAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength,
						origAddrType + shift, origAddrPlan, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1003: //������� ������ � origAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType,
						origAddrPlan + shift, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1004: //������� ������ � origAddrValue
						sms.setMessageReference(msgRef);
						sms.setOriginatingAddress(randAddrLength, origAddrType,
							origAddrPlan, randAddrValue.get());
						sms.setDestinationAddress(destAddr);
					break;
				case 1005: //������� ������ � destAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength,
						destAddrType + shift, destAddrPlan, destAddrValue);
					break;
				case 1006: //������� ������ � destAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType,
						destAddrPlan + shift, destAddrValue);
					break;
				case 1007: //������� ������ � destAddrValue
						sms.setMessageReference(msgRef);
						sms.setOriginatingAddress(origAddr);
						sms.setDestinationAddress(randAddrLength, destAddrType,
							destAddrPlan, randAddrValue.get());
					break;
				default:
					throw s;
			}
			smsId = msgStore->createSms(sms, flag);
			if (smsId == existentId)
			{
				res->addFailure(1101);
				throw s;
			}
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			*idp = 0;
			SmsUtil::clearSms(smsp);
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeDuplicateSms(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSms)
{
	TCResult* res = new TCResult(TC_STORE_DUPLICATE_SMS);
	try
	{
		SMS sms(existentSms);
		SMSId id = msgStore->createSms(sms, CREATE_NEW);
		if (id == existentId)
		{
			res->addFailure(101);
		}
		if (idp != NULL && smsp != NULL)
		{
			*idp = id;
			*smsp = sms;
		}
	}
	catch(...)
	{
		*idp = 0;
		SmsUtil::clearSms(smsp);
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::storeRejectDuplicateSms(const SMS& existentSms)
{
	TCResult* res = new TCResult(TC_STORE_REJECT_DUPLICATE_SMS);
	try
	{
		SMS sms;
		SmsUtil::setupRandomCorrectSms(&sms);
		//����������� � SMPP v3.4
		//�������� GSM 03.40 ����� 9.2.3.25 ������ ���������: TP-MR, TP-DA, OA.
		sms.setMessageReference(existentSms.getMessageReference());
		sms.setOriginatingAddress(existentSms.getOriginatingAddress());
		sms.setDestinationAddress(existentSms.getDestinationAddress());
		SMSId smsId = msgStore->createSms(sms, ETSI_REJECT_IF_PRESENT);
		res->addFailure(101);
	}
	catch(DuplicateMessageException&)
	{
		//ok
	}
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::storeReplaceCorrectSms(const SMSId existentId,
	SMS* existentSms)
{
	TCResult* res = new TCResult(TC_STORE_REPLACE_CORRECT_SMS);
	try
	{
		SMS sms;
		SmsUtil::setupRandomCorrectSms(&sms);
		//����������� � GSM 03.40
		//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
		//destination address and service_type. ��������� ������ ���� � 
		//ENROTE state.
		sms.setOriginatingAddress(existentSms->getOriginatingAddress());
		sms.setDestinationAddress(existentSms->getDestinationAddress());
		EService serviceType;
		existentSms->getEServiceType(serviceType);
		sms.setEServiceType(serviceType);
		SMSId smsId = msgStore->createSms(sms, SMPP_OVERWRITE_IF_PRESENT);
		*existentSms = sms;
		if (smsId != existentId)
		{
			res->addFailure(101);
		}
	}
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::storeReplaceSmsInFinalState(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSms)
{
	TCResult* res = new TCResult(TC_STORE_REPLACE_SMS_IN_FINAL_STATE);
	try
	{
		//����������� � GSM 03.40
		//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
		//destination address and service_type � ��������� ������ ���� � 
		//ENROTE state. � ��������� ������ ��������� ����� ���������.
		SMS sms(existentSms);
		SMSId smsId = msgStore->createSms(sms, SMPP_OVERWRITE_IF_PRESENT);
		if (smsId == existentId)
		{
			res->addFailure(101);
		}
		if (idp != NULL && smsp != NULL)
		{
			*idp = smsId;
			*smsp = sms;
		}
	}
	catch(...)
	{
		*idp = 0;
		SmsUtil::clearSms(smsp);
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::storeIncorrectSms(int num)
{
	TCResult* res = new TCResult(TC_STORE_INCORRECT_SMS);
	//����������� ���������� ��������:
	//������������ ������ DELIVERED
	//������������ ������ EXPIRED
	//������������ ������ UNDELIVERABLE
	//������������ ������ DELETED
	//���� ���������� ��� ����������
	//waitTime > validTime
	return res;
}

TCResult* MessageStoreTestCases::storeAssertSms(int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(TC_STORE_ASSERT_SMS, s.getChoice());
	SMS sms;
	for (; s.check(); s++)
	{
		try
		{
			SmsUtil::setupRandomCorrectSms(&sms);
			switch(s.value())
			{
				case 1: //������ destinationAddress
					sms.setDestinationAddress(0, 0, 0, NULL);
					break;
				case 2: //������ destinationAddress
					sms.setDestinationAddress(0, 20, 30, "*");
					break;
				case 3: //originatingAddress ������ ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 4: //destinationAddress ������ ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setDestinationAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 5: //msc ����� � originatingDescriptor ������ ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingDescriptor(MAX_ADDRESS_LENGTH + 1, addr.get(), 1, "*", 1);
					}
					break;
				case 6: //imsi ����� � originatingDescriptor ������ ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingDescriptor(1, "*", MAX_ADDRESS_LENGTH + 1, addr.get(), 1);
					}
					break;
				case 7: //msc ����� � Descriptor ������ ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						Descriptor(MAX_ADDRESS_LENGTH + 1, addr.get(), 1, "*", 1);
					}
					break;
				case 8: //imsi ����� � Descriptor ������ ������������ �����
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						Descriptor(1, "*", MAX_ADDRESS_LENGTH + 1, addr.get(), 1);
					}
					break;
				case 9: //message body ������ ������������ �����
					{
						auto_ptr<uint8_t> msgBody = rand_uint8_t(MAX_MSG_BODY_LENGTH + 1);
						sms.setMessageBody(MAX_MSG_BODY_LENGTH + 1, 20, false, msgBody.get());
					}
					break;
				case 10: //serviceType ������ ������������ �����
					{
						auto_ptr<char> type = rand_char(MAX_SERVICE_TYPE_LENGTH + 1);
						sms.setEServiceType(type.get());
					}
					break;
				default:
					throw s;
			}
			//SMSId smsId = msgStore->store(sms);
			//res->addFailure(s.value());
		}
		catch(AssertException&)
		{
			//ok
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::changeExistentSmsStateEnrouteToEnroute(
	const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_ENROUTE,
		s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			Descriptor dst;
			dst.setSmeNumber(rand0(65535));
			uint8_t failureCause = rand1(255);
			time_t nextTryTime = time(NULL);
			switch(s.value())
			{
				case 1: //������ imsi � msc (��������, SME �� ������ imsi � msc)
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					break;
				case 2: //������ imsi � msc (��������, SME �� ������ imsi � msc)
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					break;
				case 3: //imsi � msc ������ ������������ �����
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					break;
				case 4: //nextTryTime � ������� �������
					nextTryTime -= 100;
					break;
				case 5: //failureCause �� �����
					failureCause = 0;
					break;
				default:
					throw s;
			}
			//hack, ��� ������� ���������
			sms->destinationDescriptor = dst;
			sms->failureCause = failureCause;
			sms->attempts++;
			sms->lastTime = time(NULL);
			sms->nextTime = nextTryTime;
			msgStore->changeSmsStateToEnroute(id, dst, failureCause, nextTryTime);
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

//hack, ��� ������� ���������
#define SET_DELIVERED_SMS sms->state = DELIVERED; \
	sms->destinationDescriptor = dst; \
	sms->lastTime = time(NULL); \
	sms->nextTime = 0; \
	sms->failureCause = 0; \
	sms->attempts++;

//hack, ��� ������� ���������
#define SET_UNDELIVERABLE_SMS sms->state = UNDELIVERABLE; \
	sms->destinationDescriptor = dst; \
	sms->lastTime = time(NULL); \
	sms->nextTime = 0; \
	sms->failureCause = failureCause; \
	sms->attempts++;

TCResult* MessageStoreTestCases::changeExistentSmsStateEnrouteToFinal(
	const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 9);
	TCResult* res = new TCResult(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_FINAL, 
		s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			Descriptor dst;
			dst.setSmeNumber(rand0(65535));
			uint8_t failureCause = rand1(255);
			switch(s.value())
			{
				case 1: //DELIVERED, ������ imsi � msc
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					SET_DELIVERED_SMS
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 2: //DELIVERED, ������ imsi � msc
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					SET_DELIVERED_SMS
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 3: //DELIVERED, imsi � msc ������ ������������ �����
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					SET_DELIVERED_SMS
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 4: //UNDELIVERABLE, ������ imsi � msc
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					SET_UNDELIVERABLE_SMS
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 5: //UNDELIVERABLE, ������ imsi � msc
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					SET_UNDELIVERABLE_SMS
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 6: //UNDELIVERABLE, imsi � msc ������ ������������ �����
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					SET_UNDELIVERABLE_SMS
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 7: //UNDELIVERABLE, failureCause �� �����
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					failureCause = 0;
					SET_UNDELIVERABLE_SMS
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 8: //EXPIRED
					//hack, ��� ������� ���������
					sms->state = EXPIRED;
					sms->nextTime = 0;
					msgStore->changeSmsStateToExpired(id);
					break;
				case 9: //DELETED
					//hack, ��� ������� ���������
					sms->state = DELETED;
					//sms->lastTime = time(NULL);
					sms->nextTime = 0;
					msgStore->changeSmsStateToDeleted(id);
					break;
				default:
					throw s;
			}
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::changeFinalSmsStateToAny(const SMSId id, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_CHANGE_FINAL_SMS_STATE_TO_ANY, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			Descriptor dst;
			SmsUtil::setupRandomCorrectDescriptor(&dst);
			uint8_t failureCause = rand1(255);
			time_t nextTryTime = time(NULL);
			switch(s.value())
			{
				case 1: //ENROUTE
					msgStore->changeSmsStateToEnroute(id, dst, failureCause, nextTryTime);
					break;
				case 2: //DELIVERED
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 3: //UNDELIVERABLE
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 4: //EXPIRED
					msgStore->changeSmsStateToExpired(id);
					break;
				case 5: //DELETED
					msgStore->changeSmsStateToDeleted(id);
					break;
				default:
					throw s;
			}
			res->addFailure(s.value());
		}
		catch(NoSuchMessageException&)
		{
			//ok
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::replaceCorrectSms(const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 6);
	TCResult* res = new TCResult(TC_REPLACE_CORRECT_SMS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			Body body;
			SmsUtil::setupRandomCorrectBody(&body);
			uint8_t deliveryReport = rand0(255);
			time_t validTime = time(NULL) + 100;
			time_t waitTime = time(NULL) + 50;
			switch(s.value())
			{
				case 1: //������������ ��� ��������� ��������
					break;
				case 2: //�������� schedule_delivery_time ��� ���������
					waitTime = 0;
					break;
				case 3: //�������� validity_period ��� ���������
					validTime = 0;
					break;
				case 4: //������ ���� ���������
					body.setData(0, NULL);
					break;
				case 5: //������ ���� ���������
					{
						uint8_t tmp[] = {0};
						body.setData(0, tmp);
					}
					break;
				case 6: //���� ��������� ������������ �����
					{
						auto_ptr<uint8_t> data = rand_uint8_t(MAX_MSG_BODY_LENGTH);
						body.setData(MAX_MSG_BODY_LENGTH, data.get());
					}
					break;
				default:
					throw s;
			}
			//������ �����, ����� � ������� ������ �������
			uint8_t data[MAX_SHORT_MESSAGE_LENGTH];
			uint8_t len = body.getData(data);
			sms->getMessageBody().setData(len, data);
			sms->setDeliveryReport(deliveryReport);
			if (validTime != 0)
			{
				sms->setValidTime(validTime);
			}
			if (waitTime != 0)
			{
				sms->setWaitTime(waitTime);
			}
			msgStore->replaceSms(id, sms->getOriginatingAddress(),
                body, deliveryReport, validTime, waitTime);
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::replaceIncorrectSms(const SMSId id,
	const SMS& sms, int num)
{
	//��������� �������� ������ ���������� �������:
	//���� ���������� ��� ����������
	//waitTime > validTime

	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_REPLACE_INCORRECT_SMS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			Address addr(sms.getOriginatingAddress());
			Body body;
			SmsUtil::setupRandomCorrectBody(&body);
			uint8_t deliveryReport = rand0(255);
			time_t validTime = time(NULL) + 100;
			time_t waitTime = time(NULL) + 50;
			switch(s.value())
			{
				case 1: //������� ������ � TypeOfNumber
					addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
					break;
				case 2: //������� ������ � NumberingPlan
					addr.setNumberingPlan(addr.getNumberingPlan() + 1);
					break;
				case 3: //������� ������ � Value
					{
						int len = rand1(MAX_ADDRESS_VALUE_LENGTH);
						auto_ptr<char> val = rand_char(len);
						addr.setValue(len, val.get());
					}
					break;
				default:
					throw s;
			}
			msgStore->replaceSms(id, addr, body, deliveryReport, validTime, waitTime);
			res->addFailure(s.value());
		}
		catch(NoSuchMessageException&)
		{
			//ok
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::replaceFinalSms(const SMSId id, const SMS& sms)
{
	TCResult* res = new TCResult(TC_REPLACE_FINAL_SMS);
	try
	{
		Body body;
		SmsUtil::setupRandomCorrectBody(&body);
		uint8_t deliveryReport = rand0(255);
		time_t validTime = time(NULL) + 100;
		time_t waitTime = time(NULL) + 50;
		msgStore->replaceSms(id, sms.getOriginatingAddress(), body,
			deliveryReport, validTime, waitTime);
		res->addFailure(101);
	}
	catch(NoSuchMessageException&)
	{
		//ok
	}
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::loadExistentSms(const SMSId id, const SMS& sms)
{
	TCResult* res = new TCResult(TC_LOAD_EXISTENT_SMS);
	try
	{
		SMS _sms;
		msgStore->retriveSms(id, _sms);
		if (&sms == NULL || &_sms == NULL)
		{
			res->addFailure(101);
		}
		else
		{
			vector<int> tmp = SmsUtil::compareMessages(sms,_sms);
			for (int i = 0; i < tmp.size(); i++)
			{
				res->addFailure(tmp[i]);
			}
		}
	}
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::loadNonExistentSms(const SMSId id)
{
	TCResult* res = new TCResult(TC_LOAD_NON_EXISTENT_SMS);
	try
	{
		SMS sms;
		msgStore->retriveSms(id, sms);
		res->addFailure(101);
	}
	catch(NoSuchMessageException&)
	{
		//Ok
	}
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::deleteExistentSms(const SMSId id)
{
	TCResult* res = new TCResult(TC_DELETE_EXISTENT_SMS);
	try
	{
		msgStore->destroySms(id);
	}
	catch (...)
	{
		res->addFailure(100);
	}
	return res;
}
	
TCResult* MessageStoreTestCases::deleteNonExistentSms(const SMSId id)
{
	TCResult* res = new TCResult(TC_DELETE_NON_EXISTENT_SMS);
	try
	{
		msgStore->destroySms(id);
		res->addFailure(101);
	}
	catch (NoSuchMessageException&)
	{
		//Ok
	}
	catch (...)
	{
		res->addFailure(100);
	}
	return res;
}
	
}
}
}

