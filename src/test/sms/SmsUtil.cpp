#include "SmsUtil.hpp"
#include "test/util/Util.hpp"
#include <cstring>
#include <map>

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

#define __compare_int_body_tag__(tagName, errCode) \
	if ((_b1->hasIntProperty(Tag::tagName) && !_b2->hasIntProperty(Tag::tagName)) || \
		(!_b1->hasIntProperty(Tag::tagName) && _b2->hasIntProperty(Tag::tagName)) || \
		(_b1->hasIntProperty(Tag::tagName) && _b2->hasIntProperty(Tag::tagName) && \
		_b1->getIntProperty(Tag::tagName) != _b2->getIntProperty(Tag::tagName))) \
	{ res.push_back(errCode); }

#define __compare_str_body_tag__(tagName, errCode) \
	if ((_b1->hasStrProperty(Tag::tagName) && !_b2->hasStrProperty(Tag::tagName)) || \
		(!_b1->hasStrProperty(Tag::tagName) && _b2->hasStrProperty(Tag::tagName)) || \
		(_b1->hasStrProperty(Tag::tagName) && _b2->hasStrProperty(Tag::tagName) && \
		_b1->getStrProperty(Tag::tagName) != _b2->getStrProperty(Tag::tagName))) \
	{ res.push_back(errCode); }

vector<int> SmsUtil::compareMessageBodies(const Body& b1, const Body& b2)
{
	//return (b1.getBufferLength() == b2.getBufferLength() &&
	//	memcmp(b1.getBuffer(), b2.getBuffer(), b1.getBufferLength()) == 0);
	Body* _b1 = const_cast<Body*>(&b1);
	Body* _b2 = const_cast<Body*>(&b2);
	__require__(_b1 && _b2);
	vector<int> res;
	__compare_int_body_tag__(SMPP_SCHEDULE_DELIVERY_TIME, 1);
	__compare_int_body_tag__(SMPP_REPLACE_IF_PRESENT_FLAG, 2);
	__compare_int_body_tag__(SMPP_ESM_CLASS, 3);
	__compare_int_body_tag__(SMPP_DATA_CODING, 4);
	__compare_int_body_tag__(SMPP_SM_LENGTH, 5);
	__compare_int_body_tag__(SMPP_REGISTRED_DELIVERY, 6);
	__compare_int_body_tag__(SMPP_PROTOCOL_ID, 7);
	__compare_str_body_tag__(SMPP_SHORT_MESSAGE, 8);
	__compare_int_body_tag__(SMPP_PRIORITY, 9);
	__compare_int_body_tag__(SMPP_USER_MESSAGE_REFERENCE, 10);
	__compare_int_body_tag__(SMPP_USSD_SERVICE_OP, 11);
	__compare_int_body_tag__(SMPP_DEST_ADDR_SUBUNIT, 12);
	__compare_int_body_tag__(SMPP_PAYLOAD_TYPE, 13);
	__compare_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, 14);
	__compare_int_body_tag__(SMPP_MS_MSG_WAIT_FACILITIES, 15);
	__compare_int_body_tag__(SMPP_USER_RESPONSE_CODE, 16);
	__compare_int_body_tag__(SMPP_SAR_MSG_REF_NUM, 17);
	__compare_int_body_tag__(SMPP_LANGUAGE_INDICATOR, 18);
	__compare_int_body_tag__(SMPP_SAR_TOTAL_SEGMENTS, 19);
	__compare_int_body_tag__(SMPP_NUMBER_OF_MESSAGES, 20);
	__compare_str_body_tag__(SMPP_MESSAGE_PAYLOAD, 21);
	return res;
}

#define __compare__(getter, errCode) \
	if (_sms1->getter() != _sms2->getter()) { res.push_back(errCode); }

#define __compare_addr__(getter, errCode) \
	if (!compareAddresses(_sms1->getter(), _sms2->getter())) { res.push_back(errCode); }
	
#define __compare_desc__(getter, errCode) \
	if (!compareDescriptors(_sms1->getter(), _sms2->getter())) { res.push_back(errCode); }

vector<int> SmsUtil::compareMessages(const SMS& sms1, const SMS& sms2, SmsCompareFlag flag)
{
	SMS* _sms1 = const_cast<SMS*>(&sms1);
	SMS* _sms2 = const_cast<SMS*>(&sms2);
	__require__(_sms1 && _sms2);
	vector<int> res;
	__compare__(getState, 1);
	__compare__(getSubmitTime, 2);
	__compare__(getValidTime, 3);
	__compare__(getAttemptsCount, 4);
	__compare__(getLastResult, 5);
	__compare__(getLastTime, 6);
	__compare__(getNextTime, 7);
	__compare_addr__(getOriginatingAddress, 8);
	__compare_addr__(getDestinationAddress, 9);
	__compare_addr__(getDealiasedDestinationAddress, 10);
	__compare__(getMessageReference, 11);

	EService serviceType1, serviceType2;
	sms1.getEServiceType(serviceType1);
	sms2.getEServiceType(serviceType2);
	if (strcmp(serviceType1, serviceType2))
	{
		res.push_back(12);
	}

	__compare__(isArchivationRequested, 13);
	__compare__(getDeliveryReport, 14);
	__compare__(getBillingRecord, 15);
	__compare_desc__(getOriginatingDescriptor, 16);
	__compare_desc__(getDestinationDescriptor, 17);

	vector<int> tmp = compareMessageBodies(sms1.getMessageBody(),
		sms2.getMessageBody());
	for (int i = 0; i < tmp.size(); i++)
	{
		res.push_back(20 + tmp[i]);
	}
	//bool attach;

	return res;
}

void SmsUtil::setupRandomCorrectAddress(Address* addr)
{
	setupRandomCorrectAddress(addr, 1, MAX_ADDRESS_VALUE_LENGTH);
}

void SmsUtil::setupRandomCorrectAddress(Address* addr, int minLen, int maxLen)
{
	if (addr)
	{
		__require__(minLen > 0 && maxLen <= MAX_ADDRESS_VALUE_LENGTH);
		int len = rand2(minLen, maxLen);
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
		desc->setSmeNumber((uint32_t) rand0(INT_MAX));
	}
}

#define __set_int_body_tag__(tagName, value) \
	if ((mask >>= 1) & 0x1) { \
		__trace__("set_int_body_tag: " #tagName); \
		uint32_t tmp = value; \
		intMap[Tag::tagName] = tmp; \
		body->setIntProperty(Tag::tagName, tmp); \
	}

#define __set_str_body_tag__(tagName, len) \
	if ((mask >>= 1) & 0x1) { \
		__trace__("set_str_body_tag: " #tagName); \
		auto_ptr<char> str = rand_char(len); \
		strMap.insert(StrMap::value_type(Tag::tagName, str.get())); \
		body->setStrProperty(Tag::tagName, str.get()); \
	}

#define __check_int_body_tag__(tagName) \
	IntMap::const_iterator it_##tagName = intMap.find(Tag::tagName); \
	if (it_##tagName == intMap.end()) { \
		__require__(!body->hasIntProperty(Tag::tagName)); \
	} else { \
		__require__(body->hasIntProperty(Tag::tagName) && \
			body->getIntProperty(Tag::tagName) == it_##tagName->second); \
	}
	
#define __check_str_body_tag__(tagName) \
	StrMap::const_iterator it_##tagName = strMap.find(Tag::tagName); \
	if (it_##tagName == strMap.end()) { \
		__require__(!body->hasStrProperty(Tag::tagName)); \
	} else { \
		__require__(body->hasStrProperty(Tag::tagName) && \
			body->getStrProperty(Tag::tagName) == it_##tagName->second); \
	}
	
void SmsUtil::setupRandomCorrectBody(Body* body)
{
	//поля сохраняются в body случайным образом
	//даже обязательные для sms поля могут не сохраняться в БД
	auto_ptr<uint8_t> tmp = rand_uint8_t(8);
	uint64_t mask = *((uint64_t*) tmp.get());

	typedef map<const string, uint32_t> IntMap;
	typedef map<const string, const string> StrMap;
	StrMap strMap;
	IntMap intMap;

	int msgLen = rand1(MAX_SM_LENGTH);
	__set_int_body_tag__(SMPP_SCHEDULE_DELIVERY_TIME, time(NULL) + rand0(24 * 3600));
	__set_int_body_tag__(SMPP_REPLACE_IF_PRESENT_FLAG, rand0(2));
	__set_int_body_tag__(SMPP_ESM_CLASS, rand0(255));
	__set_int_body_tag__(SMPP_DATA_CODING, rand0(255));
	__set_int_body_tag__(SMPP_SM_LENGTH, msgLen);
	__set_int_body_tag__(SMPP_REGISTRED_DELIVERY, rand0(255));
	__set_int_body_tag__(SMPP_PROTOCOL_ID, rand0(255));
	__set_str_body_tag__(SMPP_SHORT_MESSAGE, msgLen);
	__set_int_body_tag__(SMPP_PRIORITY, rand0(255));
	__set_int_body_tag__(SMPP_USER_MESSAGE_REFERENCE, rand0(65535));
	__set_int_body_tag__(SMPP_USSD_SERVICE_OP, rand0(255));
	__set_int_body_tag__(SMPP_DEST_ADDR_SUBUNIT, rand0(255));
	__set_int_body_tag__(SMPP_PAYLOAD_TYPE, rand0(255));
	__set_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, rand1(64));
	__set_int_body_tag__(SMPP_MS_MSG_WAIT_FACILITIES, rand0(255));
	__set_int_body_tag__(SMPP_USER_RESPONSE_CODE, rand0(255));
	__set_int_body_tag__(SMPP_SAR_MSG_REF_NUM, rand0(65535));
	__set_int_body_tag__(SMPP_LANGUAGE_INDICATOR, rand0(255));
	__set_int_body_tag__(SMPP_SAR_TOTAL_SEGMENTS, rand0(255));
	__set_int_body_tag__(SMPP_NUMBER_OF_MESSAGES, rand0(255));
	__set_str_body_tag__(SMPP_MESSAGE_PAYLOAD, rand1(MAX_PAYLOAD_LENGTH));

	//checks
	__check_int_body_tag__(SMPP_SCHEDULE_DELIVERY_TIME);
	__check_int_body_tag__(SMPP_REPLACE_IF_PRESENT_FLAG);
	__check_int_body_tag__(SMPP_ESM_CLASS);
	__check_int_body_tag__(SMPP_DATA_CODING);
	__check_int_body_tag__(SMPP_SM_LENGTH);
	__check_int_body_tag__(SMPP_REGISTRED_DELIVERY);
	__check_int_body_tag__(SMPP_PROTOCOL_ID);
	__check_str_body_tag__(SMPP_SHORT_MESSAGE);
	__check_int_body_tag__(SMPP_PRIORITY);
	__check_int_body_tag__(SMPP_USER_MESSAGE_REFERENCE);
	__check_int_body_tag__(SMPP_USSD_SERVICE_OP);
	__check_int_body_tag__(SMPP_DEST_ADDR_SUBUNIT);
	__check_int_body_tag__(SMPP_PAYLOAD_TYPE);
	__check_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID);
	__check_int_body_tag__(SMPP_MS_MSG_WAIT_FACILITIES);
	__check_int_body_tag__(SMPP_USER_RESPONSE_CODE);
	__check_int_body_tag__(SMPP_SAR_MSG_REF_NUM);
	__check_int_body_tag__(SMPP_LANGUAGE_INDICATOR);
	__check_int_body_tag__(SMPP_SAR_TOTAL_SEGMENTS);
	__check_int_body_tag__(SMPP_NUMBER_OF_MESSAGES);
	__check_str_body_tag__(SMPP_MESSAGE_PAYLOAD);
}

#define __set_sms__(type, field, value) \
	type tmp##field = value; \
	sms->set##field(tmp##field); \
	__require__(sms->get##field() == tmp##field);

#define __set_addr_sms__(field) \
	Address tmp##field; \
	setupRandomCorrectAddress(&tmp##field); \
	sms->set##field(tmp##field); \
	__require__(compareAddresses(sms->get##field(), tmp##field));

#define __set_desc_sms__(field) \
	Descriptor tmp##field; \
	setupRandomCorrectDescriptor(&tmp##field); \
	sms->set##field(tmp##field); \
	__require__(compareDescriptors(sms->get##field(), tmp##field));
	
void SmsUtil::setupRandomCorrectSms(SMS* sms)
{
	//sms->setState(...);
	__set_sms__(time_t, SubmitTime, time(NULL) + rand2(-3600, 0));
	__set_sms__(time_t, ValidTime, time(NULL) + rand0(24 * 3600));
	//sms->setAttemptsCount(...);
	//sms->setLastResult(...);
	//sms->setLastTime(...);
	__set_sms__(time_t, NextTime, time(NULL) + rand0(24 * 3600));
	__set_addr_sms__(OriginatingAddress);
	__set_addr_sms__(DestinationAddress);
	__set_addr_sms__(DealiasedDestinationAddress);
	__set_sms__(uint16_t, MessageReference, rand0(65535));
	//eServiceType
	auto_ptr<char> serviceType = rand_char(MAX_ESERVICE_TYPE_LENGTH);
	sms->setEServiceType(serviceType.get());
	EService _serviceType;
	sms->getEServiceType(_serviceType);
	__require__(!strcmp(_serviceType, serviceType.get()));
	//needArchivate
	bool arc = rand0(3);
	sms->setArchivationRequested(arc);
	__require__(sms->isArchivationRequested() == arc);
	//SMPP v3.4, пункт 5.2.17
	//xxxxxx00 - No SMSC Delivery Receipt requested (default)
	//xxxxxx01 - SMSC Delivery Receipt requested where final delivery
	//			outcome is delivery success or failure
	//xxxxxx10 - SMSC Delivery Receipt requested where the final
	//			delivery outcome is delivery failure
	__set_sms__(uint8_t, DeliveryReport, rand0(255));
	__set_sms__(uint8_t, BillingRecord, rand0(3));
	__set_desc_sms__(OriginatingDescriptor);
	//setupRandomCorrectDescriptor(sms->getDestinationDescriptor());
	setupRandomCorrectBody(&sms->getMessageBody());
	//bool attach;
}

void SmsUtil::clearSms(SMS* sms)
{
	//sms->setState(...);
	sms->setSubmitTime(0);
	sms->setValidTime(0);
	//sms->setAttemptsCount(...);
	//sms->setLastResult(...);
	//sms->setLastTime(...);
	sms->setNextTime(0);
	sms->setOriginatingAddress(1, 0, 0, "*");
	sms->setDestinationAddress(1, 0, 0, "*");
	sms->setDealiasedDestinationAddress(1, 0, 0, "*");
	sms->setMessageReference(0);
	sms->setEServiceType("");
	sms->setArchivationRequested(false);
	sms->setDeliveryReport(0);
	sms->setBillingRecord(false);
	sms->setOriginatingDescriptor(1, "*", 0, "*", 0);
	//sms->setDestinationDescriptor(...);
	sms->getMessageBody() = Body();
	//bool attach;
}

auto_ptr<char> SmsUtil::configString(const Address& addr)
{
	char* tmp = new char[32];
	AddressValue addrVal;
	addr.getValue(addrVal);
	if (addr.getTypeOfNumber() == 1 && addr.getNumberingPlan() == 1)
	{
		sprintf(tmp, "+%s", addrVal);
	}
	else if (addr.getTypeOfNumber() == 2 && addr.getNumberingPlan() == 1)
	{
		sprintf(tmp, "%s", addrVal);
	}
	else
	{
		sprintf(tmp, ".%d.%d.%s", (int) addr.getTypeOfNumber(),
				(int) addr.getNumberingPlan(), addrVal);
	}
	return auto_ptr<char>(tmp);
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

ostream& operator<< (ostream& os, const Address& a)
{
	AddressValue addrVal;
	int addrLen = a.getValue(addrVal);
	int ton = a.getTypeOfNumber();
	int npi = a.getNumberingPlan();
	os << "{ton=" << ton << ", npi=" << npi <<
		", val=" << addrVal << "(" << addrLen << ")}";
	return os;
}

}
}
}

