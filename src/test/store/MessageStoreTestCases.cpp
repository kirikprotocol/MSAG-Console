#include "util/config/Manager.h"
#include "util/debug.h"
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
using namespace smsc::util;
using namespace smsc::util::config;

MessageStoreTestCases::MessageStoreTestCases()
{
	srand(time(NULL));
	Manager& config = Manager::getInstance();
	StoreManager::startup(config.getStoreConfig());
	msgStore = StoreManager::getMessageStore();
}

MessageStoreTestCases::~MessageStoreTestCases()
{
	StoreManager::shutdown();
}
void MessageStoreTestCases::setupRandomCorrectSM(SMS& sms)
{
	sms.setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
	int addrLength = rand1(MAX_ADDRESS_LENGTH); //задаем случайную длину адреса
	sms.setOriginatingAddress(addrLength, 20, 30, rand_char(addrLength).get());
	sms.setDestinationAddress(addrLength, 20, 30, rand_char(addrLength).get());
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
    int msgLen = rand1(MAX_MSG_BODY_LENGTH); //задаем случайную длину тела сообщения
	sms.setMessageBody(msgLen, 20, false, rand_uint8_t(msgLen).get());
}

TCResult* MessageStoreTestCases::storeCorrectSM(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_STORE_CORRECT_SM, s.getChoice());
	SMSId smsId;
	SMS sms;
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSM(sms);
			switch(s.value())
			{
				case 1: //ничего особенного
					break;
				case 2: //пустой originatingAddress
					sms.setOriginatingAddress(0, 20, 30, rand_char(1).get());
					break;
				case 3: //пустое тело сообщения
					sms.setMessageBody(0, 20, false, rand_uint8_t(1).get());
					break;
				case 4: //длинное тело сообщения
					sms.setMessageBody(MAX_MSG_BODY_LENGTH, 20, false, 
						rand_uint8_t(MAX_MSG_BODY_LENGTH).get());
					break;
				default:
					__require__(false);
			}
			smsId = msgStore->store(sms);
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			res->addFailure(s.value());;
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeIncorrectSM(SMS& existentSMS, int num)
{
	TCSelector s(num, 7);
	TCResult* res = new TCResult(TC_STORE_INCORRECT_SM, s.getChoice());
	SMS sms;
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSM(sms);
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
				case 7: //дублированное сообщение
					sms.setMessageReference(existentSMS.getMessageReference());
					sms.setRejectDuplicates(true);
					break;
				default:
					__require__(false);
			}
			SMSId smsId = msgStore->store(sms);
			res->addFailure(s.value());
		}
		catch(StoreException e)
		{
			continue;
		}
		catch(...)
		{
			res->addFailure(s.value());;
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::storeAssertSM(int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_STORE_ASSERT_SM, s.getChoice());
	SMS sms;
	int bigAddrLength = MAX_ADDRESS_LENGTH + 1;
	int msgBodyLength = MAX_MSG_BODY_LENGTH + 1;
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSM(sms);
			switch(s.value())
			{
				case 1: //пустой destinationAddress
					sms.setDestinationAddress(0, 20, 30, rand_char(1).get());
					break;
				case 2: //originatingAddress больше максимальной длины
					sms.setOriginatingAddress(bigAddrLength, 20, 30,
						rand_char(bigAddrLength).get());
					break;
				case 3: //destinationAddress больше максимальной длины
					sms.setDestinationAddress(bigAddrLength, 20, 30,
						rand_char(bigAddrLength).get());
					break;
				case 4: //message body больше максимальной длины
					sms.setMessageBody(msgBodyLength, 20, false,
						rand_uint8_t(msgBodyLength).get());
					break;
				default:
					__require__(false);
			}
			SMSId smsId = msgStore->store(sms);
			res->addFailure(s.value());
		}
		catch(AssertException e)
		{
			continue;
		}
		catch(...)
		{
			res->addFailure(s.value());;
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::setCorrectSMStatus()
{
	return false;
}

TCResult* MessageStoreTestCases::setIncorrectSMStatus()
{
	return false;
}

TCResult* MessageStoreTestCases::setNonExistentSMStatus()
{
	return false;
}

TCResult* MessageStoreTestCases::updateCorrectExistentSM()
{
	return false;
}

TCResult* MessageStoreTestCases::updateIncorrectExistentSM()
{
	return false;
}
	
TCResult* MessageStoreTestCases::updateNonExistentSM()
{
	return false;
}

TCResult* MessageStoreTestCases::deleteExistentSM()
{
	return false;
}
	
TCResult* MessageStoreTestCases::deleteNonExistentSM()
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
	catch(...)
	{
		res->addFailure(100);
	}
	return res;
}

TCResult* MessageStoreTestCases::loadNonExistentSM()
{
	TCResult* res = new TCResult(TC_LOAD_NON_EXISTENT_SM, 1);
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
	catch(...)
	{
		res->addFailure(101);
	}
	return res;
}

TCResult* MessageStoreTestCases::createBillingRecord()
{
	return false;
}

}
}
}

