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
	msgStore = StoreManager::getMessageStore();
}

MessageStoreTestCases::~MessageStoreTestCases() {}

void MessageStoreTestCases::setupRandomCorrectSM(SMS& sms)
{
	sms.setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
	int addrLength = rand1(MAX_ADDRESS_LENGTH); //������ ��������� ����� ������
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
    int msgLen = rand1(MAX_MSG_BODY_LENGTH); //������ ��������� ����� ���� ���������
	sms.setMessageBody(msgLen, 20, false, rand_uint8_t(msgLen).get());
}

void MessageStoreTestCases::clearSM(SMS& sms)
{
	sms.setState(ENROUTE); //DELIVERED, EXPIRED, UNDELIVERABLE, DELETED
	sms.setOriginatingAddress(0, 0, 0, NULL);
	sms.setDestinationAddress(1, 0, 0, "*");
	sms.setWaitTime(0);
	sms.setValidTime(0);
	sms.setSubmitTime(0);
	sms.setDeliveryTime(0);
	sms.setMessageReference(0);
	sms.setMessageIdentifier(0);
	sms.setPriority(0);
	sms.setProtocolIdentifier(0);
	sms.setStatusReportRequested(false);
	sms.setRejectDuplicates(false);
	sms.setFailureCause(0);
	sms.setMessageBody(0, 0, false, NULL);
}

TCResult* MessageStoreTestCases::storeCorrectSM(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 9);
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
				case 1: //������ ����������
					break;
				case 2: //������ originatingAddress
					sms.setOriginatingAddress(0, 0, 0, NULL);
					break;
				case 3: //������ originatingAddress
					sms.setOriginatingAddress(0, 20, 30, rand_char(1).get());
					break;
				case 4: //originatingAddress ������������ �����
					sms.setOriginatingAddress(MAX_ADDRESS_LENGTH, 20, 30, 
						rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 5: //destinationAddress ����������� �����
					sms.setDestinationAddress(1, 20, 30, rand_char(1).get());
					break;
				case 6: //destinationAddress ������������ �����
					sms.setDestinationAddress(MAX_ADDRESS_LENGTH, 20, 30, 
						rand_char(MAX_ADDRESS_LENGTH).get());
					break;
				case 7: //������ ���� ���������
					sms.setMessageBody(0, 20, false, NULL);
					break;
				case 8: //������ ���� ���������
					sms.setMessageBody(0, 20, false, rand_uint8_t(1).get());
					break;
				case 9: //���� ��������� ������������ �����
					sms.setMessageBody(MAX_MSG_BODY_LENGTH, 20, false, 
						rand_uint8_t(MAX_MSG_BODY_LENGTH).get());
					break;
				default:
					throw s;
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
			idp = NULL;
			clearSM(*smsp);
			res->addFailure(s.value());
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
				case 1: //������������ ������
					sms.setState(DELIVERED);
					break;
				case 2: //������������ ������
					sms.setState(EXPIRED);
					break;
				case 3: //������������ ������
					sms.setState(UNDELIVERABLE);
					break;
				case 4: //������������ ������
					sms.setState(DELETED);
					break;
				case 5: //���� ���������� ��� ����������
					sms.setValidTime(time(NULL) - 1);
					break;
				case 6: //waitTime > validTime
					sms.setWaitTime(time(NULL) + 200);
					sms.setValidTime(time(NULL) + 100);
					break;
				case 7: //������������� ���������
					sms.setMessageReference(existentSMS.getMessageReference());
					sms.setRejectDuplicates(true);
					break;
				default:
					throw s;
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
	TCSelector s(num, 5);
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
				case 1: //������ destinationAddress
					sms.setDestinationAddress(0, 0, 0, NULL);
					break;
				case 2: //������ destinationAddress
					sms.setDestinationAddress(0, 20, 30, rand_char(1).get());
					break;
				case 3: //originatingAddress ������ ������������ �����
					sms.setOriginatingAddress(bigAddrLength, 20, 30,
						rand_char(bigAddrLength).get());
					break;
				case 4: //destinationAddress ������ ������������ �����
					sms.setDestinationAddress(bigAddrLength, 20, 30,
						rand_char(bigAddrLength).get());
					break;
				case 5: //message body ������ ������������ �����
					sms.setMessageBody(msgBodyLength, 20, false,
						rand_uint8_t(msgBodyLength).get());
					break;
				default:
					throw s;
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
			res->addFailure(s.value());
		}
	}
	return res;
}

TCResult* MessageStoreTestCases::setCorrectSMStatus()
{
	return NULL;
}

TCResult* MessageStoreTestCases::setIncorrectSMStatus()
{
	return NULL;
}

TCResult* MessageStoreTestCases::setNonExistentSMStatus()
{
	return NULL;
}

TCResult* MessageStoreTestCases::updateCorrectExistentSM()
{
	return NULL;
}

TCResult* MessageStoreTestCases::updateIncorrectExistentSM()
{
	return NULL;
}
	
TCResult* MessageStoreTestCases::updateNonExistentSM()
{
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
				case 1: //�������������� ������� id �� ����������
					msgStore->remove(id);
					break;
				case 2: //�������� �������������� id
					msgStore->remove(0xFFFFFFFF);
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
	
TCResult* MessageStoreTestCases::loadExistentSM(SMSId id, SMS& sms)
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
					msgStore->retrive(0xFFFFFFFF, sms);
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

