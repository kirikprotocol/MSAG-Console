#include "MessageStoreTestCases.hpp"
#include "store/StoreManager.h"
#include "store/StoreExceptions.h"
#include <cstdlib>
#include <ctime>
#include <string>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::test::util;
using namespace smsc::store;
using namespace smsc::sms;

MessageStoreTestCases::MessageStoreTestCases()
	throw (smsc::store::StoreException)
{
	srand(time(NULL));
	msgStore = StoreManager::getInstance();
}

TCResult MessageStoreTestCases::storeCorrectSM(SMS* smsp, int num)
{
	TCResult res("MessageStoreTestCases::storeCorrectSM");
	SMS sms;
	for (TCSelector s(num, 4); s.check(); s++)
	{
		sms.setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
		sms.setOriginatingAddress(10, 20, 30, rand_uint8_t(10).get());
		sms.setDestinationAddress(10, 20, 30, rand_uint8_t(10).get());
		sms.setWaitTime(time(NULL));
		sms.setValidTime(time(NULL) + 24 * 3600);
		sms.setSubmitTime(time(NULL) - 1);
		sms.setDeliveryTime(0);
		sms.setMessageReference(*(rand_uint8_t(1).get()));
		sms.setMessageIdentifier(*(rand_uint8_t(1).get()));
		sms.setPriority(100);
		sms.setProtocolIdentifier(150);
		sms.setStatusReportRequested(true);
		sms.setRejectDuplicates(true);
		sms.setFailureCause(55);
		sms.setMessageBody(10, 20, false, rand_uint8_t(10).get());
		
		switch(s.value())
		{
			case 1: //пустой originatingAddress
				sms.setOriginatingAddress(0, 20, 30, rand_uint8_t(1).get());
				break;
			case 2: //пустой destinationAddress
				sms.setDestinationAddress(0, 20, 30, rand_uint8_t(1).get());
				break;
			case 3: //пустое тело сообщения
				sms.setMessageBody(0, 20, false, rand_uint8_t(1).get());
				break;
			case 4: //длинное тело сообщения
				sms.setMessageBody(256, 20, false, rand_uint8_t(256).get());
				break;
			default:
				res.addFailure(s.value());
				return res;
		}
		try
		{
			SMSId smsId = msgStore->store(sms);
		}
		catch(exception& e)
		{
			res.addFailure(s.value());;
		}
	}
	if (smsp != NULL)
	{
		*smsp = sms;
	}
	return res;
}

TCResult MessageStoreTestCases::storeIncorrectSM(SMS& existentSMS, int num)
{
	TCResult res("MessageStoreTestCases::storeIncorrectSM");
	for (TCSelector s(num, 10); s.check(); s++)
	{
		SMS sms;
		sms.setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
		sms.setOriginatingAddress(10, 20, 30, rand_uint8_t(10).get());
		sms.setDestinationAddress(10, 20, 30, rand_uint8_t(10).get());
		sms.setWaitTime(time(NULL));
		sms.setValidTime(time(NULL) + 24 * 3600);
		sms.setSubmitTime(time(NULL) - 1);
		sms.setDeliveryTime(0);
		sms.setMessageReference(*(rand_uint8_t(1).get()));
		sms.setMessageIdentifier(*(rand_uint8_t(1).get()));
		sms.setPriority(100);
		sms.setProtocolIdentifier(150);
		sms.setStatusReportRequested(true);
		sms.setRejectDuplicates(true);
		sms.setFailureCause(55);
		sms.setMessageBody(10, 20, false, rand_uint8_t(10).get());
		
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
			case 5: //сшишком длинный originatingAddress
				sms.setOriginatingAddress(11, 20, 30, rand_uint8_t(11).get());
				break;
			case 6: //destinationAddress длиннее 11 символов
				sms.setDestinationAddress(11, 20, 30, rand_uint8_t(11).get());
				break;
			case 7: //срок валидности уже закончился
				sms.setValidTime(time(NULL) - 1);
				break;
			case 8: //waitTime > validTime
				sms.setWaitTime(time(NULL) + 200);
				sms.setValidTime(time(NULL) + 100);
				break;
			case 9: //дублированное сообщение
				sms.setMessageReference(existentSMS.getMessageReference());
				sms.setRejectDuplicates(true);
				break;
			case 10: //слишком длинное message body
				sms.setMessageBody(257, 20, false, rand_uint8_t(257).get());
				break;
			case 100: //корректное сообщение
				break;
			default:
				res.addFailure(s.value());
				return res;
		}
		try
		{
			SMSId smsId = msgStore->store(sms);
			res.addFailure(s.value());
		}
		catch(StoreException e)
		{
			continue;
		}
		catch(exception& e)
		{
			res.addFailure(s.value());;
		}
	}
	return res;
}

bool MessageStoreTestCases::setCorrectSMStatus()
{
	return false;
}

bool MessageStoreTestCases::setIncorrectSMStatus()
{
	return false;
}

bool MessageStoreTestCases::setNonExistentSMStatus()
{
	return false;
}

bool MessageStoreTestCases::updateCorrectExistentSM()
{
	return false;
}

bool MessageStoreTestCases::updateIncorrectExistentSM()
{
	return false;
}
	
bool MessageStoreTestCases::updateNonExistentSM()
{
	return false;
}

bool MessageStoreTestCases::deleteExistingSM()
{
	return false;
}
	
bool MessageStoreTestCases::deleteNonExistingSM()
{
	return false;
}
	
bool MessageStoreTestCases::loadExistingSM()
{
	/*
	try
	{
		SMS* retrive(SMSId id);
	}
	catch(ResourceAllocationException& e)
	{
	}
	catch(NoSuchMessageException& e)
	{
	}
	*/
	return false;
}

bool MessageStoreTestCases::loadNonExistingSM()
{
	return false;
}

bool MessageStoreTestCases::createBillingRecord()
{
	return false;
}

}
}
}

