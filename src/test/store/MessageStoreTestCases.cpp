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
using smsc::test::sms::SmsUtil;
using smsc::util::AssertException;

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
				case 1: //ничего особенного
					break;
				case 2: //originatingAddress минимальной длины
					//согласно SMPP v3.4 должно быть 0, но мы делаем так
					sms.setOriginatingAddress(1, 10, 20, rand_char(1).get());
					break;
				case 3: //originatingAddress максимальной длины
					sms.setOriginatingAddress(MAX_ADDRESS_LENGTH, 20, 30, 
						rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 4: //destinationAddress минимальной длины
					sms.setDestinationAddress(1, 30, 40, rand_char(1).get());
					break;
				case 5: //destinationAddress максимальной длины
					sms.setDestinationAddress(MAX_ADDRESS_LENGTH, 40, 50, 
						rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 6: //пустой imsi (см. GSM 09.02 пункт 12.2) и пустой msc (???)
					sms.setOriginatingDescriptor(0, NULL, 0, NULL, 10);
					break;
				case 7: //пустой imsi (см. GSM 09.02 пункт 12.2) и пустой msc (???)
					sms.setOriginatingDescriptor(0, rand_char(1).get(),
						0, rand_char(1).get(), 20);
					break;
				case 8: //imsi и msc адреса максимальной длины
					sms.setOriginatingDescriptor(
						MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get(),
						MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get(), 30);
					break;
				case 9: //пустое тело сообщения
					sms.setMessageBody(0, 10, false, NULL);
					break;
				case 10: //пустое тело сообщения
					sms.setMessageBody(0, 20, false, rand_uint8_t(1).get());
					break;
				case 11: //тело сообщения максимальной длины
					sms.setMessageBody(MAX_MSG_BODY_LENGTH, 30, false, 
						rand_uint8_t(MAX_MSG_BODY_LENGTH).get());
					break;
				case 12: //нулевой serviceType, NULL недопустимо
                   	sms.setEServiceType("");
					break;
				case 13: //serviceType максимальной длины
                    sms.setEServiceType(rand_char(MAX_SERVICE_TYPE_LENGTH).get());
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
	TCSelector s(num, 8, 1000);
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
	//новый адрес
	int randAddrLength = rand1(MAX_ADDRESS_LENGTH);
	char* randAddrValue = rand_char(randAddrLength).get();
	for (; s.check(); s++)
	{
		SMS sms;
		try
		{
			SmsUtil::setupRandomCorrectSms(&sms);
			SMSId smsId;
			CreateMode flag = ETSI_REJECT_IF_PRESENT;
			switch(s.value())
			{
				case 1001: //разрешаю дубликаты
					flag = CREATE_NEW;
					sms = existentSms;
					break;
				case 1002: //отличие только в msgRef
					sms.setMessageReference(msgRef + 1);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddr);
					break;
				case 1003: //отличие только в origAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType + 1,
						origAddrPlan, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1004: //отличие только в origAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType,
						origAddrPlan + 1, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1005: //отличие только в origAddrValue
						sms.setMessageReference(msgRef);
						sms.setOriginatingAddress(randAddrLength, origAddrType,
							origAddrPlan, randAddrValue);
						sms.setDestinationAddress(destAddr);
					break;
				case 1006: //отличие только в destAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType + 1,
						destAddrPlan, destAddrValue);
					break;
				case 1007: //отличие только в destAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType,
						destAddrPlan + 1, destAddrValue);
					break;
				case 1008: //отличие только в destAddrValue
						sms.setMessageReference(msgRef);
						sms.setOriginatingAddress(origAddr);
						sms.setDestinationAddress(randAddrLength, destAddrType,
							destAddrPlan, randAddrValue);
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

TCResult* MessageStoreTestCases::storeRejectDuplicateSms(const SMS& existentSms)
{
	TCResult* res = new TCResult(TC_STORE_REJECT_DUPLICATE_SMS);
	try
	{
		SMS sms;
		SmsUtil::setupRandomCorrectSms(&sms);
		//Отсутствует в SMPP v3.4
		//Согласно GSM 03.40 пункт 9.2.3.25 должны совпадать: TP-MR, TP-DA, OA.
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
		//Отсутствует в GSM 03.40
		//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
		//destination address and service_type. Сообщение должно быть в 
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
		//Отсутствует в GSM 03.40
		//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
		//destination address and service_type и сообщение должно быть в 
		//ENROTE state. В противном случае создается новое сообщение.
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
	//Проверяется вызывающей стороной:
	//некорректный статус DELIVERED
	//некорректный статус EXPIRED
	//некорректный статус UNDELIVERABLE
	//некорректный статус DELETED
	//срок валидности уже закончился
	//waitTime > validTime
	return res;
}

TCResult* MessageStoreTestCases::storeAssertSms(int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_STORE_ASSERT_SMS, s.getChoice());
	SMS sms;
	int bigAddrLength = MAX_ADDRESS_LENGTH + 1;
	int msgBodyLength = MAX_MSG_BODY_LENGTH + 1;
	for (; s.check(); s++)
	{
		try
		{
			SmsUtil::setupRandomCorrectSms(&sms);
			switch(s.value())
			{
				case 1: //пустой destinationAddress
					sms.setDestinationAddress(0, 0, 0, NULL);
					break;
				case 2: //пустой destinationAddress
					sms.setDestinationAddress(0, 20, 30, rand_char(1).get());
					break;
				case 3: //originatingAddress больше максимальной длины
					sms.setOriginatingAddress(bigAddrLength, 20, 30,
						rand_char(bigAddrLength).get());
					break;
				case 4: //destinationAddress больше максимальной длины
					sms.setDestinationAddress(bigAddrLength, 20, 30,
						rand_char(bigAddrLength).get());
					break;
				case 5: //message body больше максимальной длины
					sms.setMessageBody(msgBodyLength, 20, false,
						rand_uint8_t(msgBodyLength).get());
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
				case 1: //пустой imsi и msc (например, SME не имееют imsi и msc)
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					break;
				case 2: //пустой imsi и msc (например, SME не имееют imsi и msc)
					dst.setMsc(0, rand_char(1).get());
					dst.setImsi(0, rand_char(1).get());
					break;
				case 3: //imsi и msc адреса максимальной длины
					dst.setMsc(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					dst.setImsi(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 4: //nextTryTime в прошлом времени
					nextTryTime -= 100;
					break;
				case 5: //failureCause не задан
					failureCause = 0;
					break;
				default:
					throw s;
			}
			sms->destinationDescriptor = dst; //hack, сеттер запрещен
			sms->failureCause = failureCause; //hack, сеттер запрещен
			sms->nextTime = nextTryTime; //hack, сеттер запрещен
			msgStore->changeSmsStateToEnroute(id, dst, failureCause, nextTryTime);
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

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
				case 1: //DELIVERED, пустой imsi и msc
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					sms->state = DELIVERED; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = 0; //hack, сеттер запрещен
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 2: //DELIVERED, пустой imsi и msc
					dst.setMsc(0, rand_char(1).get());
					dst.setImsi(0, rand_char(1).get());
					sms->state = DELIVERED; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = 0; //hack, сеттер запрещен
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 3: //DELIVERED, imsi и msc адреса максимальной длины
					dst.setMsc(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					dst.setImsi(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					sms->state = DELIVERED; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = 0; //hack, сеттер запрещен
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 4: //UNDELIVERABLE, пустой imsi и msc
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					sms->state = UNDELIVERABLE; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = failureCause; //hack, сеттер запрещен
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 5: //UNDELIVERABLE, пустой imsi и msc
					dst.setMsc(0, rand_char(1).get());
					dst.setImsi(0, rand_char(1).get());
					sms->state = UNDELIVERABLE; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = failureCause; //hack, сеттер запрещен
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 6: //UNDELIVERABLE, imsi и msc адреса максимальной длины
					dst.setMsc(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					dst.setImsi(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					sms->state = UNDELIVERABLE; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = failureCause; //hack, сеттер запрещен
					msgStore->changeSmsStateToUndeliverable(id, dst, failureCause);
					break;
				case 7: //UNDELIVERABLE, failureCause не задан
					dst.setMsc(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					dst.setImsi(MAX_ADDRESS_LENGTH, rand_char(MAX_ADDRESS_LENGTH).get());
					sms->state = UNDELIVERABLE; //hack, сеттер запрещен
					sms->destinationDescriptor = dst; //hack, сеттер запрещен
					sms->failureCause = 0; //hack, сеттер запрещен
					msgStore->changeSmsStateToUndeliverable(id, dst, 0);
					break;
				case 8: //EXPIRED
					sms->state = EXPIRED; //hack, сеттер запрещен
					msgStore->changeSmsStateToExpired(id);
					break;
				case 9: //DELETED
					sms->state = DELETED; //hack, сеттер запрещен
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
				case 1: //проапдейтить все возможные атрибуты
					break;
				case 2: //оставить schedule_delivery_time без изменений
					waitTime = 0;
					break;
				case 3: //оставить validity_period без изменений
					validTime = 0;
					break;
				case 4: //пустое тело сообщения
					body.setData(0, NULL);
					break;
				case 5: //пустое тело сообщения
					body.setData(0, rand_uint8_t(1).get());
					break;
				case 6: //тело сообщения максимальной длины
					body.setData(MAX_MSG_BODY_LENGTH,
						rand_uint8_t(MAX_MSG_BODY_LENGTH).get());
					break;
				default:
					throw s;
			}
			//данные новые, схема и наличие хедера прежние
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
	//Следующие проверки делает вызывающая сторона:
	//срок валидности уже закончился
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
				case 1: //отличие только в TypeOfNumber
					addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
					break;
				case 2: //отличие только в NumberingPlan
					addr.setNumberingPlan(addr.getNumberingPlan() + 1);
					break;
				case 3: //отличие только в Value
					{
						int len = rand1(MAX_ADDRESS_VALUE_LENGTH);
						addr.setValue(len, rand_char(len).get());
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
			vector<int>& tmp = *SmsUtil::compareMessages(sms, _sms).get();
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

