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

TCResult* MessageStoreTestCases::storeCorrectSM(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SM, s.getChoice());
	SMSId smsId;
	SMS sms;
	for (; s.check(); s++)
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
				res->addFailure(s.value());
				return res;
		}
		try
		{
			smsId = msgStore->store(sms);
		}
		catch(exception& e)
		{
			res->addFailure(s.value());;
		}
	}
	if (idp != NULL && smsp != NULL)
	{
		*idp = smsId;
		*smsp = sms;
	}
	return res;
}

TCResult* MessageStoreTestCases::storeIncorrectSM(SMS& existentSMS, int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(TC_STORE_INCORRECT_SM, s.getChoice());
	for (; s.check(); s++)
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
				res->addFailure(s.value());
				return res;
		}
		try
		{
			SMSId smsId = msgStore->store(sms);
			res->addFailure(s.value());
		}
		catch(StoreException e)
		{
			continue;
		}
		catch(exception& e)
		{
			res->addFailure(s.value());;
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

bool MessageStoreTestCases::deleteExistentSM()
{
	return false;
}
	
bool MessageStoreTestCases::deleteNonExistentSM()
{
	return false;
}
	
TCResult* MessageStoreTestCases::loadExistentSM(SMSId id, SMS& sms)
{
	TCResult* res = new TCResult(TC_LOAD_EXISTENT_SM, 1);
	try
	{
		SMS& _sms = msgStore->retrive(id);
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
	catch(StoreException& e)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::loadNonExistentSM()
{
	TCResult* res = new TCResult(TC_LOAD_NONEXISTENT_SM, 1);
	try
	{
		SMSId id = 0xFFFFFFFF;
		SMS& sms = msgStore->retrive(id);
		res->addFailure(100);
	}
	catch(NoSuchMessageException& e)
	{
		//Ok
	}
	catch(StoreException& e)
	{
		res->addFailure(101);
	}
	return res;
}

bool MessageStoreTestCases::createBillingRecord()
{
	return false;
}

}
}
}

