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
	bool res = a1.getLenght() == a2.getLenght() &&
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
	bool res = d1.getMscLenght() == d2.getMscLenght() &&
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
	bool res = b1.isHeaderIndicator() == b2.isHeaderIndicator() &&
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

vector<int> SmsUtil::compareMessages(const SMS& sms1, const SMS& sms2,
	SmsCompareFlag flag)
{
	vector<int> res;
	if (!(flag & IGNORE_STATE) && sms1.getState() != sms2.getState())
	{
		res.push_back(1);
	}
	if (sms1.getMessageReference() != sms2.getMessageReference())
	{
		res.push_back(2);
	}
	if (!compareAddresses(sms1.getOriginatingAddress(),
		sms2.getOriginatingAddress()))
	{
		res.push_back(3);
	}
	if (!compareAddresses(sms1.getDestinationAddress(),
		sms2.getDestinationAddress()))
	{
		res.push_back(4);
	}
	if (!(flag & IGNORE_ORIGINATING_DESCRIPTOR) &&
		!compareDescriptors(sms1.getOriginatingDescriptor(),
		sms2.getOriginatingDescriptor()))
	{
		res.push_back(5);
	}
	if (!(flag & IGNORE_DESTINATION_DESCRIPTOR) &&
		!compareDescriptors(sms1.getDestinationDescriptor(),
		sms2.getDestinationDescriptor()))
	{
		res.push_back(6);
	}
	if (sms1.getWaitTime() != sms2.getWaitTime())
	{
		res.push_back(7);
	}
	if (sms1.getValidTime() != sms2.getValidTime())
	{
		res.push_back(8);
	}
	if (sms1.getSubmitTime() != sms2.getSubmitTime())
	{
		res.push_back(9);
	}
	//совпадение с точностью до 1-ой секунды
	if (!(flag & IGNORE_LAST_TIME) &&
		abs(sms1.getLastTime() - sms2.getLastTime()) > 1)
	{
		res.push_back(10);
	}
	if (!(flag & IGNORE_NEXT_TIME) && sms1.getNextTime() != sms2.getNextTime())
	{
		res.push_back(11);
	}
	if (sms1.getPriority() != sms2.getPriority())
	{
		res.push_back(12);
	}
	if (sms1.getProtocolIdentifier() != sms2.getProtocolIdentifier())
	{
		res.push_back(13);
	}
	if (sms1.getDeliveryReport() != sms2.getDeliveryReport())
	{
		res.push_back(14);
	}
	if (!(flag & IGNORE_ARCHIVATION_REQUESTED) &&
		sms1.isArchivationRequested() != sms2.isArchivationRequested())
	{
		res.push_back(15);
	}
	if (!(flag & IGNORE_FAILURE_CAUSE) &&
		sms1.getFailureCause() != sms2.getFailureCause())
	{
		res.push_back(16);
	}
	if (!(flag & IGNORE_ATTEMPTS_COUNT) &&
		sms1.getAttemptsCount() != sms2.getAttemptsCount())
	{
		res.push_back(17);
	}
	if (!compareMessageBodies(sms1.getMessageBody(), sms2.getMessageBody()))
	{
		res.push_back(18);
	}
	char type1[MAX_ESERVICE_TYPE_LENGTH + 1];
	char type2[MAX_ESERVICE_TYPE_LENGTH + 1];
	sms1.getEServiceType(type1);
	sms2.getEServiceType(type2);
	if (strcmp(type1, type2))
	{
		res.push_back(19);
	}
	if (sms1.getReceiptSmsId() != sms2.getReceiptSmsId())
	{
		res.push_back(20);
	}
	if (sms1.getEsmClass() != sms2.getEsmClass())
	{
		res.push_back(21);
	}
	return res;
}

void SmsUtil::setupRandomCorrectAddress(Address* addr)
{
	if (addr)
	{
		int len = rand1(MAX_ADDRESS_VALUE_LENGTH);
		auto_ptr<char> val = rand_char(len);
		addr->setTypeOfNumber((uint8_t) rand0(255));
		addr->setNumberingPlan((uint8_t) rand0(255));
		addr->setValue(len, val.get());
	}
}
	
void SmsUtil::setupRandomCorrectDescriptor(Descriptor* desc)
{
	if (desc)
	{
		int len = rand1(MAX_ADDRESS_VALUE_LENGTH);
		auto_ptr<char> mscAddr = rand_char(len);
		auto_ptr<char> imsiAddr = rand_char(len);
		desc->setMsc(len, mscAddr.get());
		desc->setImsi(len, imsiAddr.get());
		desc->setSmeNumber((uint32_t) rand0(65535));
	}
}
	
void SmsUtil::setupRandomCorrectBody(Body* body)
{
	if (body)
	{
		int len = rand1(MAX_SHORT_MESSAGE_LENGTH);
		auto_ptr<uint8_t> data = rand_uint8_t(len);
		body->setData(len, data.get());
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
	auto_ptr<char> tmp = rand_char(rand1(MAX_ESERVICE_TYPE_LENGTH));
	sms->setEServiceType(tmp.get());
	sms->setReceiptSmsId(rand0(INT_MAX));
	sms->setEsmClass((uint8_t) rand0(255));
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
	sms->setReceiptSmsId(0);
	sms->setEsmClass(0);
}

bool ltAddress::operator() (const Address& a1, const Address& a2) const
{
	if (a1.getTypeOfNumber() != a2.getTypeOfNumber())
	{
		return a1.getTypeOfNumber() < a2.getTypeOfNumber();
	}
	if (a1.getNumberingPlan() != a2.getNumberingPlan())
	{
		return a1.getNumberingPlan() < a2.getNumberingPlan();
	}
	if (a1.getLenght() != a2.getLenght())
	{
		return a1.getLenght() < a2.getLenght();
	}
	AddressValue val1, val2;
	a1.getValue(val1);
	a2.getValue(val2);
	return memcmp(val1, val2, a1.getLenght()) < 0;
}

}
}
}

