#include "util/config/Manager.h"
#include "util/debug.h"
#include "MessageStoreTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "store/StoreManager.h"
#include "store/StoreExceptions.h"
#include <cstdlib>
#include <ctime>
#include <sstream>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::test::util; //TCSelector, TCResult
using namespace smsc::test::sms; //constants, SmsUtil
using namespace smsc::store; //StoreManager, CreateMode
using namespace smsc::sms; //constants, SMSId, SMS, AddressValue, EService, ...
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::AssertException;

MessageStoreTestCases::MessageStoreTestCases(MessageStore* _msgStore)
	: msgStore(_msgStore)
{
	__require__(msgStore);
}

Category& MessageStoreTestCases::getLog()
{
	static Category& log = Logger::getCategory("MessageStoreTestCases");
	return log;
}

#define __set_str__(setter, len) \
	auto_ptr<char> tmp_##setter = rand_char(len); \
	sms.setter(tmp_##setter.get());

#define __set_int_body_tag__(tagName, value) \
	sms.getMessageBody().setIntProperty(Tag::tagName, value);

#define __set_str_body_tag__(tagName, len) \
	auto_ptr<char> tmp_##tagName = rand_char(len); \
	sms.getMessageBody().setStrProperty(Tag::tagName, tmp_##tagName.get());
	
TCResult* MessageStoreTestCases::storeCorrectSms(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 15);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SMS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			SMS sms;
			SmsUtil::setupRandomCorrectSms(&sms);
			switch(s.value())
			{
				case 1: //ничего особенного
					break;
				case 2: //originatingAddress минимальной длины
					//согласно SMPP v3.4 должно быть 0, но мы делаем 1
					sms.setOriginatingAddress(1, 10, 20, "*");
					break;
				case 3: //originatingAddress максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setOriginatingAddress(MAX_ADDRESS_LENGTH, 20, 30,
							addr.get());
					}
					break;
				case 4: //destinationAddress минимальной длины
					sms.setDestinationAddress(1, 30, 40, "*");
					break;
				case 5: //destinationAddress максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setDestinationAddress(MAX_ADDRESS_LENGTH, 40, 50, 
							addr.get());
					}
					break;
				case 6: //dealiasedDestinationAddress минимальной длины
					sms.setDealiasedDestinationAddress(1, 30, 40, "*");
					break;
				case 7: //dealiasedDestinationAddress максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setDealiasedDestinationAddress(MAX_ADDRESS_LENGTH,
							40, 50, addr.get());
					}
					break;
				case 8: //пустой imsi (см. GSM 09.02 пункт 12.2) и пустой msc (???)
					sms.setOriginatingDescriptor(0, NULL, 0, NULL, 10);
					break;
				case 9: //пустой imsi (см. GSM 09.02 пункт 12.2) и пустой msc (???)
					sms.setOriginatingDescriptor(0, "*", 0, "*", 20);
					break;
				case 10: //imsi и msc адреса максимальной длины
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setOriginatingDescriptor(
							MAX_ADDRESS_LENGTH, mscAddr.get(),
							MAX_ADDRESS_LENGTH, imsiAddr.get(), 30);
					}
					break;
				case 11: //пустое тело сообщения
					sms.getMessageBody() = Body();
					break;
				case 12: //short_message и message_payload максимальной длины
					{
                        __set_int_body_tag__(SMPP_SM_LENGTH, MAX_SM_LENGTH);
						__set_str_body_tag__(SMPP_SHORT_MESSAGE, MAX_SM_LENGTH);
						__set_str_body_tag__(SMPP_MESSAGE_PAYLOAD, MAX_PAYLOAD_LENGTH);
					}
					break;
				case 13: //пустой serviceType, NULL недопустимо
                   	sms.setEServiceType("");
					break;
				case 14: //serviceType максимальной длины
					{
						auto_ptr<char> type = rand_char(MAX_SERVICE_TYPE_LENGTH);
						sms.setEServiceType(type.get());
					}
					break;
				case 15: //receipted_message_id максимальной длины
					{
						__set_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, MAX_MSG_ID_LENGTH);
					}
					break;
				default:
					throw s;
			}
			SMSId smsId = msgStore->getNextId();
			msgStore->createSms(sms, smsId, CREATE_NEW);
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			error();
			*idp = 0;
			SmsUtil::clearSms(smsp);
			res->addFailure(s.value());
		}
	}
	debug(res);
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
	//новый адрес
	int randAddrLength = rand1(MAX_ADDRESS_VALUE_LENGTH);
	auto_ptr<char> randAddrValue = rand_char(randAddrLength);
	for (; s.check(); s++)
	{
		try
		{
			SMS sms;
			SmsUtil::setupRandomCorrectSms(&sms);
			CreateMode flag = ETSI_REJECT_IF_PRESENT;
			uint8_t shift = rand1(254);
			switch(s.value())
			{
				case 1001: //отличие только в msgRef
					sms.setMessageReference(msgRef + shift);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddr);
					break;
				case 1002: //отличие только в origAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength,
						origAddrType + shift, origAddrPlan, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1003: //отличие только в origAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType,
						origAddrPlan + shift, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1004: //отличие только в origAddrValue
						sms.setMessageReference(msgRef);
						sms.setOriginatingAddress(randAddrLength, origAddrType,
							origAddrPlan, randAddrValue.get());
						sms.setDestinationAddress(destAddr);
					break;
				case 1005: //отличие только в destAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength,
						destAddrType + shift, destAddrPlan, destAddrValue);
					break;
				case 1006: //отличие только в destAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType,
						destAddrPlan + shift, destAddrValue);
					break;
				case 1007: //отличие только в destAddrValue
						sms.setMessageReference(msgRef);
						sms.setOriginatingAddress(origAddr);
						sms.setDestinationAddress(randAddrLength, destAddrType,
							destAddrPlan, randAddrValue.get());
					break;
				default:
					throw s;
			}
			SMSId smsId = msgStore->getNextId();
			msgStore->createSms(sms, smsId, flag);
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
			error();
			*idp = 0;
			SmsUtil::clearSms(smsp);
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::storeDuplicateSms(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSms)
{
	TCResult* res = new TCResult(TC_STORE_DUPLICATE_SMS);
	try
	{
		SMS sms(existentSms);
		SMSId id = msgStore->getNextId();
		msgStore->createSms(sms, id, CREATE_NEW);
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
		error();
		*idp = 0;
		SmsUtil::clearSms(smsp);
		res->addFailure(100);
	}
	debug(res);
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
		SMSId smsId = msgStore->getNextId();
		msgStore->createSms(sms, smsId, ETSI_REJECT_IF_PRESENT);
		res->addFailure(101);
	}
	catch(DuplicateMessageException&)
	{
		//ok
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::storeReplaceCorrectSms(SMSId* idp, SMS* existentSms)
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
		sms.setDealiasedDestinationAddress(existentSms->getDealiasedDestinationAddress());
		EService serviceType;
		existentSms->getEServiceType(serviceType);
		sms.setEServiceType(serviceType);
		SMSId smsId = msgStore->getNextId();
		msgStore->createSms(sms, smsId, SMPP_OVERWRITE_IF_PRESENT);
		*idp = smsId;
		*existentSms = sms;
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::storeReplaceSmsInFinalState(SMSId* idp, SMS* smsp,
	const SMS& existentSms)
{
	TCResult* res = new TCResult(TC_STORE_REPLACE_SMS_IN_FINAL_STATE);
	try
	{
		//Отсутствует в GSM 03.40
		//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
		//destination address and service_type и сообщение должно быть в 
		//ENROTE state. В противном случае создается новое сообщение.
		SMS sms(existentSms);
		SMSId smsId = msgStore->getNextId();
		msgStore->createSms(sms, smsId, SMPP_OVERWRITE_IF_PRESENT);
		if (idp != NULL && smsp != NULL)
		{
			*idp = smsId;
			*smsp = sms;
		}
	}
	catch(...)
	{
		error();
		*idp = 0;
		SmsUtil::clearSms(smsp);
		res->addFailure(100);
	}
	debug(res);
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
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::storeAssertSms(int num)
{
	TCSelector s(num, 14);
	TCResult* res = new TCResult(TC_STORE_ASSERT_SMS, s.getChoice());
	SMS sms;
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
					sms.setDestinationAddress(0, 20, 30, "*");
					break;
				case 3: //пустой dealiasedDestinationAddress
					sms.setDealiasedDestinationAddress(0, 20, 30, "*");
					break;
				case 4: //originatingAddress больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 5: //destinationAddress больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setDestinationAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 6: //dealiasedDestinationAddress больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setDealiasedDestinationAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 7: //msc адрес в originatingDescriptor больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingDescriptor(MAX_ADDRESS_LENGTH + 1, addr.get(), 1, "*", 1);
					}
					break;
				case 8: //imsi адрес в originatingDescriptor больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingDescriptor(1, "*", MAX_ADDRESS_LENGTH + 1, addr.get(), 1);
					}
					break;
				case 9: //msc адрес в Descriptor больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						Descriptor(MAX_ADDRESS_LENGTH + 1, addr.get(), 1, "*", 1);
					}
					break;
				case 10: //imsi адрес в Descriptor больше максимальной длины
					{
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						Descriptor(1, "*", MAX_ADDRESS_LENGTH + 1, addr.get(), 1);
					}
					break;
				case 11: //short_message больше максимальной длины
					{
						__set_int_body_tag__(SMPP_SM_LENGTH, MAX_SM_LENGTH + 1);
						__set_str_body_tag__(SMPP_SHORT_MESSAGE, MAX_SM_LENGTH + 1);
					}
					break;
				case 12: //message_payload больше максимальной длины
					{
						__set_int_body_tag__(SMPP_MESSAGE_PAYLOAD, MAX_PAYLOAD_LENGTH + 1);
					}
					break;
				case 13: //serviceType больше максимальной длины
					{
						__set_str__(setEServiceType, MAX_SERVICE_TYPE_LENGTH + 1);
					}
					break;
				case 14: //receipted_message_id больше максимальной длины
					{
						__set_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, MAX_MSG_ID_LENGTH + 1);
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
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
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
			uint8_t lastResult = rand1(255);
			time_t nextTryTime = time(NULL) + rand0(3600);
			switch(s.value())
			{
				case 1: //пустой imsi и msc (например, SME не имееют imsi и msc)
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					break;
				case 2: //пустой imsi и msc (например, SME не имееют imsi и msc)
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					break;
				case 3: //imsi и msc адреса максимальной длины
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					break;
				case 4: //nextTryTime в прошлом времени
					nextTryTime -= rand0(3600);
					break;
				case 5: //lastResult не задан
					lastResult = 0;
					break;
				default:
					throw s;
			}
			msgStore->changeSmsStateToEnroute(id, dst, lastResult, nextTryTime);
			sms->setNextTime(nextTryTime);
			//hack, все сеттеры запрещены
			sms->destinationDescriptor = dst;
			sms->lastResult = lastResult;
			sms->attempts++;
			sms->lastTime = time(NULL);
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

//hack, все сеттеры запрещены
#define SET_DELIVERED_SMS sms->state = DELIVERED; \
	sms->destinationDescriptor = dst; \
	sms->lastTime = time(NULL); \
	sms->setNextTime(0); \
	sms->lastResult = 0; \
	sms->attempts++;

//hack, все сеттеры запрещены
#define SET_UNDELIVERABLE_SMS sms->state = UNDELIVERABLE; \
	sms->destinationDescriptor = dst; \
	sms->lastTime = time(NULL); \
	sms->setNextTime(0); \
	sms->lastResult = lastResult; \
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
			SmsUtil::setupRandomCorrectDescriptor(&dst);
			dst.setSmeNumber(rand0(65535));
			uint8_t lastResult = rand1(255);
			switch(s.value())
			{
				case 1: //DELIVERED, пустой imsi и msc
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					msgStore->changeSmsStateToDelivered(id, dst);
					SET_DELIVERED_SMS
					break;
				case 2: //DELIVERED, пустой imsi и msc
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					msgStore->changeSmsStateToDelivered(id, dst);
					SET_DELIVERED_SMS
					break;
				case 3: //DELIVERED, imsi и msc адреса максимальной длины
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					msgStore->changeSmsStateToDelivered(id, dst);
					SET_DELIVERED_SMS
					break;
				case 4: //UNDELIVERABLE, пустой imsi и msc
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 5: //UNDELIVERABLE, пустой imsi и msc
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 6: //UNDELIVERABLE, imsi и msc адреса максимальной длины
					{
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 7: //UNDELIVERABLE, lastResult не задан
					lastResult = 0;
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 8: //EXPIRED
					msgStore->changeSmsStateToExpired(id);
					sms->setNextTime(0);
					//hack, все сеттеры запрещены
					sms->state = EXPIRED;
					//sms->lastTime = time(NULL);
					break;
				case 9: //DELETED
					msgStore->changeSmsStateToDeleted(id);
					sms->setNextTime(0);
					//hack, все сеттеры запрещены
					sms->state = DELETED;
					//sms->lastTime = time(NULL);
					break;
				default:
					throw s;
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
			uint8_t lastResult = rand1(255);
			time_t nextTryTime = time(NULL);
			switch(s.value())
			{
				case 1: //ENROUTE
					msgStore->changeSmsStateToEnroute(id, dst, lastResult, nextTryTime);
					break;
				case 2: //DELIVERED
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 3: //UNDELIVERABLE
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
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
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::replaceCorrectSms(const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 6);
	TCResult* res = new TCResult(TC_REPLACE_CORRECT_SMS, s.getChoice());
	/*
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
					{
						uint8_t tmp[] = {0};
						body.setData(0, tmp);
					}
					break;
				case 6: //тело сообщения максимальной длины
					{
						auto_ptr<uint8_t> data = rand_uint8_t(MAX_SM_LENGTH);
						body.setData(MAX_SM_LENGTH, data.get());
					}
					break;
				default:
					throw s;
			}
			msgStore->replaceSms(id, sms->getOriginatingAddress(),
                body, deliveryReport, validTime, waitTime);
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
		}
		catch(...)
		{
			error();
			res->addFailure(s.value());
		}
	}
	*/
	res->addFailure(100);
	debug(res);
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
	/*
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
			error();
			res->addFailure(s.value());
		}
	}
	*/
	res->addFailure(100);
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::replaceFinalSms(const SMSId id, const SMS& sms)
{
	TCResult* res = new TCResult(TC_REPLACE_FINAL_SMS);
	/*
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
		error();
		res->addFailure(100);
	}
	*/
	res->addFailure(100);
	debug(res);
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
			vector<int> tmp = SmsUtil::compareMessages(sms, _sms);
			for (int i = 0; i < tmp.size(); i++)
			{
				res->addFailure(tmp[i]);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
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
		error();
		res->addFailure(100);
	}
	debug(res);
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
		error();
		res->addFailure(100);
	}
	debug(res);
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
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* MessageStoreTestCases::checkReadyForRetrySms(const vector<SMSId*>& ids,
	const vector<SMS*>& sms, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_CHECK_READY_FOR_RETRY_SMS, s.getChoice());
	//Получить времена
	bool found = false;
	time_t minNextTime, middleNextTime, maxNextTime;
	for (int i = 0; i < sms.size(); i++)
	{
		if (sms[i]->getState() != ENROUTE)
		{
			continue;
		}
		if (!sms[i]->getNextTime())
		{
			continue;
		}
		if (!found)
		{
			found = true;
			minNextTime = sms[i]->getNextTime();
			middleNextTime = sms[i]->getNextTime();
			maxNextTime = sms[i]->getNextTime();
			continue;
		}
		if (sms[i]->getNextTime() < minNextTime)
		{
			middleNextTime = minNextTime;
			minNextTime = sms[i]->getNextTime();
			continue;
		}
		if (sms[i]->getNextTime() > maxNextTime)
		{
			middleNextTime = maxNextTime;
			maxNextTime = sms[i]->getNextTime();
			continue;
		}
	}
	//MessageStore::getNextRetryTime()
	try
	{
		time_t nextTime = msgStore->getNextRetryTime();
		if (!found && nextTime != 0)
		{
			res->addFailure(1);
		}
		if (found && nextTime != minNextTime)
		{
			res->addFailure(2);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	//сравнить списки MessageStore::getReadyForRetry()
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //меньше minNextTime
					compareReadyForRetrySmsList(ids, sms, minNextTime - 1, res, 10);
					break;
				case 2: //равно middleNextTime
					compareReadyForRetrySmsList(ids, sms, middleNextTime, res, 20);
					break;
				case 3: //равно какому-то среднему времени
					compareReadyForRetrySmsList(ids, sms,
						(minNextTime + maxNextTime) / 2, res, 30);
					break;
				case 4: //больше maxNextTime
					compareReadyForRetrySmsList(ids, sms, maxNextTime + 1, res, 40);
					break;
				default:
					throw s;
			}
		}
		catch(...)
		{
			error();
			res->addFailure(200);
		}
	}
	debug(res);
	return res;
}

void MessageStoreTestCases::compareReadyForRetrySmsList(const vector<SMSId*>& ids, 
	const vector<SMS*>& sms, time_t time, TCResult* res, int shift)
{
	//выбрать и отсортировать ids
	typedef map<SMSId, time_t> IdMap;
	IdMap enroteIds;
	for (int i = 0; i < sms.size(); i++)
	{
		if (sms[i]->getState() == ENROUTE && sms[i]->getNextTime() > 0 &&
			sms[i]->getNextTime() <= time)
		{
			enroteIds[*ids[i]] = sms[i]->getNextTime();
		}
	}
	//сверить списки
	SMSId id;
	time_t prevTime = 0;
	IdIterator* it = msgStore->getReadyForRetry(time);
	bool sortOrderOk = true;
	bool idFound = true;
	while (it->getNextId(id))
	{
		IdMap::iterator idIt = enroteIds.find(id);
		if (idIt == enroteIds.end())
		{
			idFound = false;
			continue;
		}
		time_t curTime = idIt->second;
		if (curTime < prevTime)
		{
			sortOrderOk = false;
			continue;
		}
		prevTime = curTime;
		enroteIds.erase(idIt);
//log.debug("dbid = %d, msid = %d", id, enroteIds[i].smsId);
	}
	if (!idFound)
	{
		res->addFailure(shift + 1);
	}
	if (!sortOrderOk)
	{
		res->addFailure(shift + 2);
	}
	if (enroteIds.size())
	{
//log.debug("dbid!!! = %d", id);
		res->addFailure(shift + 3);
	}
	delete it;
}

}
}
}

