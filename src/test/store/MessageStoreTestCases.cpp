#include "util/config/Manager.h"
#include "util/debug.h"
#include "MessageStoreTestCases.hpp"
#include "store/StoreManager.h"
#include "store/StoreExceptions.h"
#include <cstdlib>
#include <ctime>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::test::util;
using namespace smsc::store;
using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::util::config;

MessageStoreTestCases::MessageStoreTestCases()
{
	srand(time(NULL));
	msgStore = StoreManager::getMessageStore();
}

MessageStoreTestCases::~MessageStoreTestCases() {}

void MessageStoreTestCases::setupRandomCorrectSM(SMS* sms)
{
	sms->setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
	int addrLength = rand1(MAX_ADDRESS_LENGTH); //задаем случайную длину адреса
	sms->setOriginatingAddress(addrLength, 20, 30, rand_char(addrLength).get());
	sms->setDestinationAddress(addrLength, 20, 30, rand_char(addrLength).get());
	sms->setWaitTime(time(NULL));
	sms->setValidTime(time(NULL) + 24 * 3600);
	sms->setSubmitTime(time(NULL) - 1);
	sms->setDeliveryTime(0);
	sms->setMessageReference(*(rand_uint8_t(1).get()));
	sms->setMessageIdentifier(*(rand_uint8_t(1).get()));
	sms->setPriority(100);
	sms->setProtocolIdentifier(150);
	sms->setStatusReportRequested(true);
	sms->setRejectDuplicates(true);
	sms->setFailureCause(55);
    int msgLen = rand1(MAX_MSG_BODY_LENGTH); //задаем случайную длину тела сообщения
	sms->setMessageBody(msgLen, 20, false, rand_uint8_t(msgLen).get());
}

void MessageStoreTestCases::clearSM(SMS* sms)
{
	sms->setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
	sms->setOriginatingAddress(0, 0, 0, NULL);
	sms->setDestinationAddress(1, 0, 0, "*");
	sms->setWaitTime(0);
	sms->setValidTime(0);
	sms->setSubmitTime(0);
	sms->setDeliveryTime(0);
	sms->setMessageReference(0);
	sms->setMessageIdentifier(0);
	sms->setPriority(0);
	sms->setProtocolIdentifier(0);
	sms->setStatusReportRequested(false);
	sms->setRejectDuplicates(false);
	sms->setFailureCause(0);
	sms->setMessageBody(0, 0, false, NULL);
}

TCResult* MessageStoreTestCases::storeCorrectSM(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 9);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SM, s.getChoice());
	for (; s.check(); s++)
	{
		SMS sms;
		try
		{
			setupRandomCorrectSM(&sms);
			switch(s.value())
			{
				case 1: //ничего особенного
					break;
				case 2: //пустой originatingAddress
					sms.setOriginatingAddress(0, 0, 0, NULL);
					break;
				case 3: //пустой originatingAddress
					sms.setOriginatingAddress(0, 20, 30, rand_char(1).get());
					break;
				case 4: //originatingAddress максимальной длины
					sms.setOriginatingAddress(MAX_ADDRESS_LENGTH, 20, 30, 
						rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 5: //destinationAddress минимальной длины
					sms.setDestinationAddress(1, 20, 30, rand_char(1).get());
					break;
				case 6: //destinationAddress максимальной длины
					sms.setDestinationAddress(MAX_ADDRESS_LENGTH, 20, 30, 
						rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 7: //пустое тело сообщения
					sms.setMessageBody(0, 20, false, NULL);
					break;
				case 8: //пустое тело сообщения
					sms.setMessageBody(0, 20, false, rand_uint8_t(1).get());
					break;
				case 9: //тело сообщения максимальной длины
					sms.setMessageBody(MAX_MSG_BODY_LENGTH, 20, false, 
						rand_uint8_t(MAX_MSG_BODY_LENGTH).get());
					break;
				default:
					throw s;
			}
			SMSId smsId = msgStore->store(sms);
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			idp = NULL;
			clearSM(smsp);
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeCorrectSM(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSMS, int num)
{
	TCSelector s(num, 7, 1000);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SM, s.getChoice());
	//messageReference
	uint8_t msgRef = existentSMS.getMessageReference();
	//originatingAddress
	const Address& origAddr = existentSMS.getOriginatingAddress();
	char* origAddrValue = new char[MAX_ADDRESS_VALUE_LENGTH + 1];
	uint8_t origAddrLength = origAddr.getValue(origAddrValue);
	uint8_t origAddrType = origAddr.getTypeOfNumber();
	uint8_t origAddrPlan = origAddr.getNumberingPlan();
	//destinationAddress
	const Address& destAddr = existentSMS.getDestinationAddress();
	char* destAddrValue = new char[MAX_ADDRESS_VALUE_LENGTH + 1];
	uint8_t destAddrLength = destAddr.getValue(destAddrValue);
	uint8_t destAddrType = destAddr.getTypeOfNumber();
	uint8_t destAddrPlan = destAddr.getNumberingPlan();
	//random address
	int addrLength = rand1(MAX_ADDRESS_LENGTH); //задаем случайную длину адреса
	char* addrValue = rand_char(addrLength).get();
	for (; s.check(); s++)
	{
		SMS sms;
		try
		{
			setupRandomCorrectSM(&sms);
			sms.setRejectDuplicates(true);
			switch(s.value())
			{
				case 1001: //отличие только в msgRef
					sms.setMessageReference(msgRef + 1);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddr);
					break;
				case 1002: //отличие только в origAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType + 1,
						origAddrPlan, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1003: //отличие только в origAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType,
						origAddrPlan + 1, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1004: //отличие только в origAddrValue
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(addrLength, origAddrType,
						origAddrPlan, addrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 1005: //отличие только в destAddrType
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType + 1,
						destAddrPlan, destAddrValue);
					break;
				case 1006: //отличие только в destAddrPlan
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType,
						destAddrPlan + 1, destAddrValue);
					break;
				case 1007: //отличие только в destAddrValue
					sms.setMessageReference(msgRef + 1);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(addrLength, destAddrType,
						destAddrPlan, addrValue);
					break;
				default:
					throw s;
			}
			delete[] origAddrValue;
			delete[] destAddrValue;
			SMSId smsId = msgStore->store(sms);
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
			idp = NULL;
			clearSM(smsp);
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeReplaceSM(SMSId smsId, SMS* sms)
{
	TCResult* res = new TCResult(TC_STORE_REPLACE_SM);
	try
	{
		uint8_t msgRef = sms->getMessageReference();
		Address origAddr = sms->getOriginatingAddress(); //return by value
		Address destAddr = sms->getDestinationAddress(); //return by value
		setupRandomCorrectSM(sms);
		//GSM 03.40 пункт 9.2.3.25
		sms->setMessageReference(msgRef);
		sms->setOriginatingAddress(origAddr);
		sms->setDestinationAddress(destAddr);
		sms->setRejectDuplicates(false);
		SMSId newId = msgStore->store(*sms);
		if (newId != smsId)
		{
			res->addFailure(101);
		}
	}
	catch(DuplicateMessageException&)
	{
		res->addFailure(102);
	}
	catch(...)
	{
		res->addFailure(103);
	}
	return res;
}

TCResult* MessageStoreTestCases::storeRejectDuplicateSM(const SMS& existentSMS)
{
	TCResult* res = new TCResult(TC_STORE_REJECT_DUPLICATE_SM);
	try
	{
		SMS sms;
		setupRandomCorrectSM(&sms);
		//GSM 03.40 пункт 9.2.3.25
		sms.setMessageReference(existentSMS.getMessageReference());
		sms.setOriginatingAddress(existentSMS.getOriginatingAddress());
		sms.setDestinationAddress(existentSMS.getDestinationAddress());
		sms.setRejectDuplicates(true);
		SMSId smsId = msgStore->store(sms);
		res->addFailure(100);
	}
	catch(DuplicateMessageException&)
	{
		//ok
	}
	catch(...)
	{
		res->addFailure(101);
	}
	return res;
}

TCResult* MessageStoreTestCases::storeIncorrectSM(int num)
{
	TCSelector s(num, 6);
	TCResult* res = new TCResult(TC_STORE_INCORRECT_SM, s.getChoice());
	SMS sms;
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSM(&sms);
			switch(s.value())
			{
				case 1: //некорректный статус
					sms.setState(DELIVERED);
					break;
				case 2: //некорректный статус
					sms.setState(EXPIRED);
					break;
				case 3: //некорректный статус
					sms.setState(UNDELIVERABLE);
					break;
				case 4: //некорректный статус
					sms.setState(DELETED);
					break;
				case 5: //срок валидности уже закончился
					sms.setValidTime(time(NULL) - 1);
					break;
				case 6: //waitTime > validTime
					sms.setWaitTime(time(NULL) + 200);
					sms.setValidTime(time(NULL) + 100);
					break;
				default:
					throw s;
			}
			SMSId smsId = msgStore->store(sms);
			res->addFailure(s.value());
		}
		catch(StoreException&)
		{
			continue;
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeAssertSM(int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_STORE_ASSERT_SM, s.getChoice());
	SMS sms;
	int bigAddrLength = MAX_ADDRESS_LENGTH + 1;
	int msgBodyLength = MAX_MSG_BODY_LENGTH + 1;
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSM(&sms);
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
			SMSId smsId = msgStore->store(sms);
			res->addFailure(s.value());
		}
		catch(AssertException&)
		{
			continue;
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::setCorrectSMStatus(SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_SET_CORRECT_SM_STATUS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			time_t curTime;
			time(&curTime);
			switch(s.value())
			{
				case 1:
					sms->setState(DELIVERED);
					sms->setDeliveryTime(curTime - 100);
					sms->setFailureCause(1);
					break;
				case 2:
					sms->setState(EXPIRED);
					sms->setDeliveryTime(curTime + 100);
					sms->setFailureCause(2);
					break;
				case 3:
					sms->setState(UNDELIVERABLE);
					sms->setDeliveryTime(curTime - 100);
					sms->setFailureCause(3);
					break;
				case 4:
					sms->setState(DELETED);
					sms->setDeliveryTime(curTime + 100);
					sms->setFailureCause(4);
					break;
				default:
					throw s;
			}
			msgStore->update(id, sms->getState(), sms->getDeliveryTime(),
				sms->getFailureCause());
		}
		catch(NoSuchMessageException&)
		{
			res->addFailure(101);
		}
		catch(StorageException&)
		{
			res->addFailure(102);
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::setIncorrectSMStatus(SMSId id)
{
	TCResult* res = new TCResult(TC_SET_INCORRECT_SM_STATUS);
	try
	{
		time_t curTime;
		time(&curTime);
		msgStore->update(id, ENROUTE, curTime, 0);
		res->addFailure(100);
	}
	catch(NoSuchMessageException&)
	{
		res->addFailure(101);
	}
	catch(StorageException&)
	{
		//ok;
	}
	catch(...)
	{
		res->addFailure(102);
	}
	return res;
}

TCResult* MessageStoreTestCases::setNonExistentSMStatus(SMSId id, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_SET_NON_EXISTENT_SM_STATUS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			time_t curTime;
			time(&curTime);
			switch (s.value())
			{
				case 1:
					msgStore->update(id, DELIVERED, curTime - 100, 1);
					break;
				case 2:
					msgStore->update(id, EXPIRED, curTime + 100, 2);
					break;
				case 3:
					msgStore->update(id, UNDELIVERABLE, curTime - 100, 3);
					break;
				case 4:
					msgStore->update(id, DELETED, curTime + 100, 4);
					break;
				case 5: //заведомо несуществующий id
					msgStore->update(0xFFFFFFFFFFFFFFFF, DELIVERED, curTime - 100, 1);
					break;
				default:
					throw s;
			}
			res->addFailure(s.value());
		}
		catch (NoSuchMessageException&)
		{
			//Ok
		}
		catch (StorageException&)
		{
			res->addFailure(s.value());
		}
		catch (...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::replaceCorrectSM(SMSId id, SMS* sms, int num)
{
	/*
	TCSelector s(num, 7);
	TCResult* res = new TCResult(TC_REPLACE_CORRECT_SM, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			SMS _sms(sms);
			switch(s.value())
			{
				case 1: //без изменений
					break;
				case 2:
					//изменяю все, но реально должно измениться только 
					//schedule_delivery_time, validity_period,
					//sm_length, short_message (SMPP v3.4, 4.10.1)
					setupRandomCorrectSM(_sms);
					//originatingAddress и id должны быть как у оригинального сообщения
					_sms.setOriginatingAddress(sms.getOriginatingAddress());
					sms.setWaitTime(_sms.getWaitTime());
					sms.setValidTime(_sms.getValidTime());
					{
						Body body;
						const Body& origBody = sms.getMessageBody();
						const Body& newBody = _sms.getMessageBody();
						uint8_t* data = new uint8_t[MAX_SHORT_MESSAGE_LENGTH];
						uint8_t len = newBody.getData(data);
						body.setData(len, data); //данные новые
						delete[] data;
						//схема и наличие хедера прежние
						body.setCodingScheme(origBody.getCodingScheme()); 
						body.setHeaderIndicator(origBody.isHeaderIndicator());
						sms.setMessageBody(body);
					}
					break;
				case 3: //оставить schedule_delivery_time без изменений
					_sms.setWaitTime(0);
					break;
				case 4: //оставить validity_period без изменений
					_sms.setValidTime(0);
					break;
				case 5: //пустое тело сообщения
					_sms.setMessageBody(0, 20, false, NULL);
					sms.setMessageBody(_sms.getMessageBody());
					break;
				case 6: //пустое тело сообщения
					_sms.setMessageBody(0, 20, false, rand_uint8_t(1).get());
					sms.setMessageBody(_sms.getMessageBody());
					break;
				case 7: //тело сообщения максимальной длины
					_sms.setMessageBody(MAX_MSG_BODY_LENGTH, 20, false, 
						rand_uint8_t(MAX_MSG_BODY_LENGTH).get());
					sms.setMessageBody(_sms.getMessageBody());
					break;
				default:
					throw s;
			}
			msgStore->replace(id, _sms);
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
	*/
	return NULL;
}

TCResult* MessageStoreTestCases::replaceIncorrectSM(SMSId id, const SMS& sms, int num)
{
	/*
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_REPLACE_INCORRECT_SM, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			SMS _sms(sms);
			switch(s.value())
			{
				case 1: //срок валидности уже закончился
					_sms.setValidTime(time(NULL) - 1);
					break;
				case 2: //waitTime > validTime
					_sms.setWaitTime(time(NULL) + 200);
					_sms.setValidTime(time(NULL) + 100);
					break;
				default:
					throw s;
			}
			msgStore->replace(id, _sms);
		}
		catch (NoSuchMessageException&)
		{
			res->addFailure(s.value());
		}
		catch(StorageException&)
		{
			//ok
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
	*/
	return NULL;
}
	
TCResult* MessageStoreTestCases::replaceNonExistentSM(SMSId id, int num)
{
	/*
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_REPLACE_NON_EXISTENT_SM, s.getChoice());
	SMS sms;
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSM(sms);
			switch(s.value())
			{
				case 1: //несуществующий id
					msgStore->replace(0xFFFFFFFFFFFFFFFF, sms);
					break;
				case 2: //неправильный originatingAddress
					{
						int addrLen = rand0(MAX_ADDRESS_LENGTH);
						sms.setOriginatingAddress(addrLen, 20, 30,
							rand_char(addrLen).get());
					}
					msgStore->replace(id, sms);
					break;
				default:
					throw s;
			}
			res->addFailure(s.value());
		}
		catch (NoSuchMessageException&)
		{
			//ok
		}
		catch(...)
		{
			res->addFailure(s.value());
		}
	}
	return res;
	*/
	return NULL;
}

TCResult* MessageStoreTestCases::deleteExistentSM(SMSId id)
{
	TCResult* res = new TCResult(TC_DELETE_EXISTENT_SM);
	try
	{
		msgStore->remove(id);
	}
	catch (NoSuchMessageException&)
	{
		res->addFailure(101);
	}
	catch (StorageException&)
	{
		res->addFailure(102);
	}
	catch (...)
	{
		res->addFailure(100);
	}
	return res;
}
	
TCResult* MessageStoreTestCases::deleteNonExistentSM(SMSId id, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_DELETE_NON_EXISTENT_SM, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch (s.value())
			{
				case 1: //предполагается данного id не существует
					msgStore->remove(id);
					break;
				case 2: //заведомо несуществующий id
					msgStore->remove(0xFFFFFFFFFFFFFFFF);
					break;
				default:
					throw s;
			}
			res->addFailure(101);
		}
		catch (NoSuchMessageException&)
		{
			//Ok
		}
		catch (StorageException&)
		{
			res->addFailure(102);
		}
		catch (...)
		{
			res->addFailure(100);
		}
	}
	return res;
}
	
TCResult* MessageStoreTestCases::loadExistentSM(SMSId id, const SMS& sms)
{
	TCResult* res = new TCResult(TC_LOAD_EXISTENT_SM);
	try
	{
		SMS _sms;
		msgStore->retrive(id, _sms);
		if (&sms == NULL || &_sms == NULL)
		{
			res->addFailure(101);
		}
		else
		{
			if (sms.getState() != _sms.getState())
			{
				res->addFailure(102);
			}
			if (!smUtil.compareAddresses(sms.getOriginatingAddress(),
				_sms.getOriginatingAddress()))
			{
				res->addFailure(103);
			}
			if (!smUtil.compareAddresses(sms.getDestinationAddress(),
				_sms.getDestinationAddress()))
			{
				res->addFailure(104);
			}
			if (sms.getWaitTime() != _sms.getWaitTime())
			{
				res->addFailure(105);
			}
			if (sms.getValidTime() != _sms.getValidTime())
			{
				res->addFailure(106);
			}
			if (sms.getSubmitTime() != _sms.getSubmitTime())
			{
				res->addFailure(107);
			}
			if (sms.getDeliveryTime() != _sms.getDeliveryTime())
			{
				res->addFailure(108);
			}
			if (sms.getMessageReference() != _sms.getMessageReference())
			{
				res->addFailure(109);
			}
			if (sms.getMessageIdentifier() != _sms.getMessageIdentifier())
			{
				res->addFailure(110);
			}
			if (sms.getPriority() != _sms.getPriority())
			{
				res->addFailure(111);
			}
			if (sms.getProtocolIdentifier() != _sms.getProtocolIdentifier())
			{
				res->addFailure(112);
			}
			if (sms.isStatusReportRequested() != _sms.isStatusReportRequested())
			{
				res->addFailure(113);
			}
			if (sms.isRejectDuplicates() != _sms.isRejectDuplicates())
			{
				res->addFailure(114);
			}
			if (sms.getFailureCause() != _sms.getFailureCause())
			{
				res->addFailure(115);
			}
			if (!smUtil.compareMessageBody(sms.getMessageBody(),
				_sms.getMessageBody()))
			{
				res->addFailure(116);
			}
		}
	}
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::loadNonExistentSM(SMSId id, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_LOAD_NON_EXISTENT_SM, s.getChoice());
	for (; s.check(); s++)
	{
		SMS sms;
		try
		{
			switch (s.value())
			{
				case 1:
					msgStore->retrive(id, sms);
					break;
				case 2:
					msgStore->retrive(0xFFFFFFFFFFFFFFFF, sms);
					break;
				default:
					throw s;
			}
			res->addFailure(101);
		}
		catch(NoSuchMessageException&)
		{
			//Ok
		}
		catch (StorageException&)
		{
			res->addFailure(102);
		}
		catch(...)
		{
			res->addFailure(100);
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::createBillingRecord()
{
	return NULL;
}

}
}
}

