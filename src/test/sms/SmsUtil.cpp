#include "SmsUtil.hpp"
#include "test/util/Util.hpp"
#include <cstring>

namespace smsc {
namespace test {
namespace sms {

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::util;

bool SmsUtil::compareAddresses(const Address& a1, const Address& a2)
{
	bool res = &a1 != NULL && &a2 != NULL &&
		a1.getLenght() == a2.getLenght() &&
		a1.getTypeOfNumber() == a2.getTypeOfNumber() &&
		a1.getNumberingPlan() == a2.getNumberingPlan();
	if (res)
	{
		AddressValue val1, val2;
		a1.getValue(val1);
		a2.getValue(val2);
		res &= memcmp(val1, val2, a1.getLenght()) == 0;
	}
	return res;
}
	
bool SmsUtil::compareDescriptors(const Descriptor& d1, const Descriptor& d2)
{
	bool res = &d1 != NULL && &d2 != NULL &&
		d1.getMscLenght() == d2.getMscLenght() &&
		d1.getImsiLenght() == d2.getImsiLenght() &&
		d1.getSmeNumber() == d2.getSmeNumber();
	if (res)
	{
		AddressValue val1, val2;
		d1.getMsc(val1);
		d2.getMsc(val2);
		res &= memcmp(val1, val2, d1.getMscLenght()) == 0;
	}
	if (res)
	{
		AddressValue val1, val2;
		d1.getImsi(val1);
		d2.getImsi(val2);
		res &= memcmp(val1, val2, d1.getImsiLenght()) == 0;
	}
	return res;
}

bool SmsUtil::compareMessageBodies(const Body& b1, const Body& b2)
{
	bool res = &b1 != NULL && &b2 != NULL &&
		b1.isHeaderIndicator() == b2.isHeaderIndicator() &&
		b1.getCodingScheme() == b2.getCodingScheme() &&
		b1.getDataLenght() == b2.getDataLenght();
	if (res)
	{
		SMSData d1, d2;
		b1.getData(d1);
		b2.getData(d2);
		res &= memcmp(d1, d2, b1.getDataLenght()) == 0;
	}
	return res;
}

auto_ptr< vector<int> > SmsUtil::compareMessages(const SMS& sms1, const SMS& sms2)
{
	vector<int>* res = new vector<int>;
	if (sms1.getState() != sms2.getState())
	{
		res->push_back(1);
	}
	if (!compareAddresses(sms1.getOriginatingAddress(),
		sms2.getOriginatingAddress()))
	{
		res->push_back(2);
	}
	if (!compareAddresses(sms1.getDestinationAddress(),
		sms2.getDestinationAddress()))
	{
		res->push_back(3);
	}
	if (!compareDescriptors(sms1.getOriginatingDescriptor(),
		sms2.getOriginatingDescriptor()))
	{
		res->push_back(4);
	}
	if (!compareDescriptors(sms1.getDestinationDescriptor(),
		sms2.getDestinationDescriptor()))
	{
		res->push_back(5);
	}
	if (sms1.getWaitTime() != sms2.getWaitTime())
	{
		res->push_back(6);
	}
	if (sms1.getValidTime() != sms2.getValidTime())
	{
		res->push_back(7);
	}
	if (sms1.getSubmitTime() != sms2.getSubmitTime())
	{
		res->push_back(8);
	}
	if (sms1.getLastTime() != sms2.getLastTime())
	{
		res->push_back(9);
	}
	if (sms1.getNextTime() != sms2.getNextTime())
	{
		res->push_back(10);
	}
	if (sms1.getPriority() != sms2.getPriority())
	{
		res->push_back(11);
	}
	if (sms1.getMessageReference() != sms2.getMessageReference())
	{
		res->push_back(12);
	}
	if (sms1.getProtocolIdentifier() != sms2.getProtocolIdentifier())
	{
		res->push_back(13);
	}
	if (sms1.getDeliveryReport() != sms2.getDeliveryReport())
	{
		res->push_back(14);
	}
	if (sms1.isArchivationRequested() != sms2.isArchivationRequested())
	{
		res->push_back(15);
	}
	if (sms1.getFailureCause() != sms2.getFailureCause())
	{
		res->push_back(16);
	}
	if (sms1.getAttemptsCount() != sms2.getAttemptsCount())
	{
		res->push_back(17);
	}
	if (!compareMessageBodies(sms1.getMessageBody(), sms2.getMessageBody()))
	{
		res->push_back(18);
	}
	char type1[MAX_ESERVICE_TYPE_LENGTH + 1];
	char type2[MAX_ESERVICE_TYPE_LENGTH + 1];
	sms1.getEServiceType(type1);
	sms2.getEServiceType(type2);
	if (strcmp(type1, type2))
	{
		res->push_back(19);
	}
	return auto_ptr< vector<int> >(res);
}

void SmsUtil::setupRandomCorrectAddress(Address* addr)
{
	if (addr)
	{
		int len = rand1(MAX_ADDRESS_VALUE_LENGTH);
		addr->setTypeOfNumber((uint8_t) rand0(255));
		addr->setNumberingPlan((uint8_t) rand0(255));
		addr->setValue(len, rand_char(len).get());
	}
}
	
void SmsUtil::setupRandomCorrectDescriptor(Descriptor* desc)
{
	if (desc)
	{
		int len = rand1(MAX_ADDRESS_VALUE_LENGTH);
		desc->setMsc(len, rand_char(len).get());
		desc->setImsi(len, rand_char(len).get());
		desc->setSmeNumber((uint32_t) rand0(65535));
	}
}
	
void SmsUtil::setupRandomCorrectBody(Body* body)
{
	if (body)
	{
		int len = rand1(MAX_SHORT_MESSAGE_LENGTH);
		body->setData(len, rand_uint8_t(len).get());
		body->setCodingScheme((uint8_t) rand0(255));
		body->setHeaderIndicator((bool) rand0(1));
	}
}

void SmsUtil::setupRandomCorrectSms(SMS* sms)
{
	//sms->setState();
	setupRandomCorrectAddress(&sms->getOriginatingAddress());
	setupRandomCorrectAddress(&sms->getDestinationAddress());
	setupRandomCorrectDescriptor(&sms->getOriginatingDescriptor());
	//sms->setDestinationDescriptor();
	sms->setWaitTime(time(NULL));
	sms->setValidTime(time(NULL) + 24 * 3600);
	sms->setSubmitTime(time(NULL) - 1);
	//sms->setLastTime();
	//sms->setNextTime();
	sms->setPriority((uint8_t) rand0(255));
	sms->setMessageReference((uint8_t) rand0(255));
	sms->setProtocolIdentifier((uint8_t) rand0(255));
	//SMPP v3.4, пункт 5.2.17
	//xxxxxx00 - No SMSC Delivery Receipt requested (default)
	//xxxxxx01 - SMSC Delivery Receipt requested where final delivery
	//			outcome is delivery success or failure
	//xxxxxx10 - SMSC Delivery Receipt requested where the final
	//			delivery outcome is delivery failure
	sms->setDeliveryReport((uint8_t) rand0(2));
	sms->setArchivationRequested(true); //!!! архивировать все сообщения
	//sms->setFailureCause();
	//sms->setAttemptsCount();
	setupRandomCorrectBody(&sms->getMessageBody());
	sms->setEServiceType(rand_char(rand0(MAX_ESERVICE_TYPE_LENGTH)).get());
}

void SmsUtil::clearSms(SMS* sms)
{
	//sms->setState();
	sms->setOriginatingAddress(1, 0, 0, "*");
	sms->setDestinationAddress(1, 0, 0, "*");
	sms->setOriginatingDescriptor(1, "*", 0, "*", 0);
	//sms->setDestinationDescriptor();
	sms->setWaitTime(0);
	sms->setValidTime(0);
	sms->setSubmitTime(0);
	//sms->setLastTime();
	//sms->setNextTime();
	sms->setPriority(0);
	sms->setMessageReference(0);
	sms->setProtocolIdentifier(0);
	sms->setDeliveryReport(0);
	sms->setArchivationRequested(false);
	//sms->setFailureCause();
	//sms->setAttemptsCount();
	sms->setMessageBody(0, 0, false, NULL);
	sms->setEServiceType("");
}

}
}
}

