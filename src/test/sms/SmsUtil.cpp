#include "SmsUtil.hpp"
#include "test/util/Util.hpp"
#include <cstring>
#include <map>
#include <sstream>

namespace smsc {
namespace test {
namespace sms {

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::util;
using smsc::test::sms::operator<<;

/*
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
*/

#define __compare_int_body_tag__(tagName, errCode) \
	if (_b1->hasIntProperty(Tag::tagName) && !_b2->hasIntProperty(Tag::tagName)) { \
		__trace2__("%s: %d != NULL", #tagName, _b1->getIntProperty(Tag::tagName)); \
		res.push_back(errCode); \
	} else if (!_b1->hasIntProperty(Tag::tagName) && _b2->hasIntProperty(Tag::tagName)) { \
		__trace2__("%s: NULL != %d", #tagName, _b2->getIntProperty(Tag::tagName)); \
		res.push_back(errCode); \
	} else if (_b1->hasIntProperty(Tag::tagName) && _b2->hasIntProperty(Tag::tagName) && \
		_b1->getIntProperty(Tag::tagName) != _b2->getIntProperty(Tag::tagName)) { \
	   __trace2__("%s: %d != %d", #tagName, _b1->getIntProperty(Tag::tagName), _b2->getIntProperty(Tag::tagName)); \
	   res.push_back(errCode); \
	}

#define __compare_str_body_tag__(tagName, errCode) \
	if (_b1->hasStrProperty(Tag::tagName) && !_b2->hasStrProperty(Tag::tagName)) { \
	   __trace2__("%s: %s != NULL", #tagName, _b1->getStrProperty(Tag::tagName).c_str()); \
	   res.push_back(errCode); \
	} else if (!_b1->hasStrProperty(Tag::tagName) && _b2->hasStrProperty(Tag::tagName)) { \
	   __trace2__("%s: NULL != %s", #tagName, _b2->getStrProperty(Tag::tagName).c_str()); \
	   res.push_back(errCode); \
	} else if (_b1->hasStrProperty(Tag::tagName) && _b2->hasStrProperty(Tag::tagName) && \
		_b1->getStrProperty(Tag::tagName) != _b2->getStrProperty(Tag::tagName)) { \
	   __trace2__("%s: %s != %s", #tagName, _b1->getStrProperty(Tag::tagName).c_str(), _b2->getStrProperty(Tag::tagName).c_str()); \
	   res.push_back(errCode); \
	}

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
	if (_sms1->getter() != _sms2->getter()) { \
		ostringstream s1, s2; \
		s1 << _sms1->getter(); \
		s2 << _sms2->getter(); \
		__trace2__("%s: %s != %s", #getter, s1.str().c_str(), s2.str().c_str()); \
		res.push_back(errCode); \
	}

#define __compare_int__(getter, errCode, accuracy) \
	if (abs(_sms1->getter() - _sms2->getter()) > accuracy) { \
		ostringstream s1, s2; \
		s1 << _sms1->getter(); \
		s2 << _sms2->getter(); \
		__trace2__("%s: %s != %s", #getter, s1.str().c_str(), s2.str().c_str()); \
		res.push_back(errCode); \
	}

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
	__compare_int__(getLastTime, 6, 1); //точность сравнения = 1 секунда
	__compare__(getNextTime, 7);
	__compare__(getOriginatingAddress, 8);
	__compare__(getDestinationAddress, 9);
	__compare__(getDealiasedDestinationAddress, 10);
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
	__compare__(getOriginatingDescriptor, 16);
	__compare__(getDestinationDescriptor, 17);

	vector<int> tmp = compareMessageBodies(sms1.getMessageBody(),
		sms2.getMessageBody());
	for (int i = 0; i < tmp.size(); i++)
	{
		res.push_back(20 + tmp[i]);
	}
	//bool attach;

	return res;
}

#define __set_int__(type, field, value) \
	type tmp##field = value; \
	p->set##field(tmp##field); \
	if (check) { \
		__require__(p->get##field() == tmp##field); \
	}

#define __set_bool__(field, value) \
	bool tmp##field = value; \
	p->set##field(tmp##field); \
	if (check) { \
		__require__(p->is##field() == tmp##field); \
	}

#define __set_str__(field, length) \
	int len##field = length; \
	auto_ptr<char> str##field = rand_char(len##field); \
	p->set##field(len##field, str##field.get()); \
	if (check) { \
		char tmp##field[len##field + 10]; \
		p->get##field(tmp##field); \
		__require__(!strcmp(tmp##field, str##field.get())); \
	}

#define __set_str2__(field, length) \
	int len##field = length; \
	auto_ptr<char> str##field = rand_char(len##field); \
	p->set##field(str##field.get()); \
	if (check) { \
		char tmp##field[len##field + 10]; \
		p->get##field(tmp##field); \
		__require__(!strcmp(tmp##field, str##field.get())); \
	}

#define __set_addr__(field) \
	Address tmp##field; \
	setupRandomCorrectAddress(&tmp##field, check); \
	p->set##field(tmp##field); \
	if (check) { \
		__require__(p->get##field()== tmp##field); \
	}

#define __set_desc__(field) \
	Descriptor tmp##field; \
	setupRandomCorrectDescriptor(&tmp##field, check); \
	p->set##field(tmp##field); \
	if (check) { \
		__require__(p->get##field() == tmp##field); \
	}

void SmsUtil::setupRandomCorrectAddress(Address* addr, int minLen, int maxLen, bool check)
{
	__require__(addr);
	__require__(minLen > 0 && maxLen <= MAX_ADDRESS_VALUE_LENGTH);
	Address* p = addr;
	//set & check fields
	__set_int__(uint8_t, TypeOfNumber, rand0(255));
	__set_int__(uint8_t, NumberingPlan, rand0(255));
	__set_str__(Value, rand2(minLen, maxLen));
}
	
void SmsUtil::setupRandomCorrectAddress(Address* addr, bool check)
{
	setupRandomCorrectAddress(addr, 1, MAX_ADDRESS_VALUE_LENGTH, check);
}

void SmsUtil::setupRandomCorrectDescriptor(Descriptor* desc, bool check)
{
	__require__(desc);
	Descriptor* p = desc;
	//set & check fields
	__set_str__(Msc, rand1(MAX_ADDRESS_VALUE_LENGTH));
	__set_str__(Imsi, rand1(MAX_ADDRESS_VALUE_LENGTH));
	__set_int__(uint32_t, SmeNumber, rand0(INT_MAX));
}

#define __set_int_body_tag__(tagName, value) \
	if (mask & (pos <<= 1)) { \
		__trace2__("set_int_body_tag: " #tagName ", pos = 0x%llx", pos); \
		uint32_t tmp = value; \
		body->setIntProperty(Tag::tagName, tmp); \
		if (check) { \
			intMap[Tag::tagName] = tmp; \
		} \
	}

#define __set_str_body_tag__(tagName, length) \
	if (mask & (pos <<= 1)) { \
		__trace2__("set_str_body_tag: " #tagName ", pos = 0x%llx", pos); \
		auto_ptr<char> str = rand_char(length); \
		body->setStrProperty(Tag::tagName, str.get()); \
		if (check) { \
			strMap.insert(StrMap::value_type(Tag::tagName, str.get())); \
		} \
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
	
void SmsUtil::setupRandomCorrectBody(Body* body, uint64_t mask, bool check)
{
	__require__(body);
	//поля сохраняются в body случайным образом
	//даже обязательные для sms поля могут не сохраняться в БД
	auto_ptr<uint8_t> tmp = rand_uint8_t(8);
	mask &= *((uint64_t*) tmp.get());
	uint64_t pos = 0x1;
	__trace2__("mask = 0x%llx", mask);

	typedef map<const string, uint32_t> IntMap;
	typedef map<const string, const string> StrMap;
	StrMap strMap;
	IntMap intMap;

	//set fields
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
	//check fileds
	if (check)
	{
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
}

void SmsUtil::setupRandomCorrectSms(SMS* sms, uint64_t mask, bool check)
{
	__require__(sms);
	SMS* p = sms;
	//set & check fields
	//sms->setState(...);
	__set_int__(time_t, SubmitTime, time(NULL) + rand2(-3600, 0));
	__set_int__(time_t, ValidTime, time(NULL) + rand0(24 * 3600));
	//sms->setAttemptsCount(...);
	//sms->setLastResult(...);
	//sms->setLastTime(...);
	__set_int__(time_t, NextTime, time(NULL) + rand0(24 * 3600));
	__set_addr__(OriginatingAddress);
	__set_addr__(DestinationAddress);
	__set_addr__(DealiasedDestinationAddress);
	__set_int__(uint16_t, MessageReference, rand0(65535));
	__set_str2__(EServiceType, MAX_ESERVICE_TYPE_LENGTH);
	__set_bool__(ArchivationRequested, rand0(3));
	//SMPP v3.4, пункт 5.2.17
	//xxxxxx00 - No SMSC Delivery Receipt requested (default)
	//xxxxxx01 - SMSC Delivery Receipt requested where final delivery
	//			outcome is delivery success or failure
	//xxxxxx10 - SMSC Delivery Receipt requested where the final
	//			delivery outcome is delivery failure
	__set_int__(uint8_t, DeliveryReport, rand0(255));
	__set_int__(uint8_t, BillingRecord, rand0(3));
	__set_desc__(OriginatingDescriptor);
	//setupRandomCorrectDescriptor(sms->getDestinationDescriptor(), check);
	setupRandomCorrectBody(&sms->getMessageBody(), mask, check);
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
	return a1 < a2;
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

bool operator== (const Address& a1, const Address& a2)
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

bool operator!= (const Address& a1, const Address& a2)
{
	return !operator==(a1, a2);
}

bool operator< (const Address& a1, const Address& a2)
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

auto_ptr<char> str(const Address& addr)
{
	char* tmp = new char[32];
	AddressValue addrVal;
	addr.getValue(addrVal);
	sprintf(tmp, ".%d.%d.%s", (int) addr.getTypeOfNumber(),
		(int) addr.getNumberingPlan(), addrVal);
	return auto_ptr<char>(tmp);
}

ostream& operator<< (ostream& os, const Descriptor& d)
{
	os << "{msc=" << d.msc << "(" << d.mscLength <<
		"), imsi=" << d.imsi << "(" << d.imsiLength <<
		"), sme=" << d.sme << "}";
	return os;
}

bool operator==(const Descriptor& d1, const Descriptor& d2)
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

bool operator!=(const Descriptor& d1, const Descriptor& d2)
{
	return !operator==(d1, d2);
}

}
}
}

