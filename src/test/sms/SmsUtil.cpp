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

#define __compare__(getter, errCode) \
	if (!mask[pos++] && _sms1->getter() != _sms2->getter()) { \
		ostringstream s1, s2; \
		s1 << _sms1->getter(); \
		s2 << _sms2->getter(); \
		__trace2__("%s: %s != %s", #getter, s1.str().c_str(), s2.str().c_str()); \
		res.push_back(errCode); \
	}

#define __compare_str__(getter, errCode) \
	if (!mask[pos++]) { \
		if (strcmp(sms1.getter(), sms2.getter())) { \
			__trace2__(#getter ": %s != %s", sms1.getter(), sms2.getter()); \
			res.push_back(errCode); \
		} \
	}

#define __compare_int__(getter, errCode, accuracy) \
	if (!mask[pos++] && abs(_sms1->getter() - _sms2->getter()) > accuracy) { \
		ostringstream s1, s2; \
		s1 << (int) _sms1->getter(); \
		s2 << (int) _sms2->getter(); \
		__trace2__("%s: %s != %s", #getter, s1.str().c_str(), s2.str().c_str()); \
		res.push_back(errCode); \
	}

#define __compare_int_body_tag__(tagName, errCode) \
	if (!mask[pos++]) { \
		if (_sms1->hasIntProperty(Tag::tagName) && !_sms2->hasIntProperty(Tag::tagName)) { \
			__trace2__(#tagName ": %d != NULL", _sms1->getIntProperty(Tag::tagName)); \
			res.push_back(errCode); \
		} else if (!_sms1->hasIntProperty(Tag::tagName) && _sms2->hasIntProperty(Tag::tagName)) { \
			__trace2__(#tagName ": NULL != %d", _sms2->getIntProperty(Tag::tagName)); \
			res.push_back(errCode); \
		} else if (_sms1->hasIntProperty(Tag::tagName) && _sms2->hasIntProperty(Tag::tagName) && \
			_sms1->getIntProperty(Tag::tagName) != _sms2->getIntProperty(Tag::tagName)) { \
			__trace2__(#tagName ": %d != %d", _sms1->getIntProperty(Tag::tagName), _sms2->getIntProperty(Tag::tagName)); \
			res.push_back(errCode); \
		} \
	}

#define __compare_str_body_tag__(tagName, errCode) \
	if (!mask[pos++]) { \
		if (_sms1->hasStrProperty(Tag::tagName) && !_sms2->hasStrProperty(Tag::tagName)) { \
			__trace2__(#tagName ": %s != NULL", _sms1->getStrProperty(Tag::tagName).c_str()); \
			res.push_back(errCode); \
		} else if (!_sms1->hasStrProperty(Tag::tagName) && _sms2->hasStrProperty(Tag::tagName)) { \
			__trace2__(#tagName ": NULL != %s", _sms2->getStrProperty(Tag::tagName).c_str()); \
			res.push_back(errCode); \
		} else if (_sms1->hasStrProperty(Tag::tagName) && _sms2->hasStrProperty(Tag::tagName) && \
			_sms1->getStrProperty(Tag::tagName) != _sms2->getStrProperty(Tag::tagName)) { \
			__trace2__(#tagName ": %s != %s", _sms1->getStrProperty(Tag::tagName).c_str(), _sms2->getStrProperty(Tag::tagName).c_str()); \
			res.push_back(errCode); \
		} \
	}

#define __compare_bin_body_tag__(tagName, errCode) \
	if (!mask[pos++]) { \
		unsigned len1, len2; \
		if (_sms1->hasBinProperty(Tag::tagName) && !_sms2->hasBinProperty(Tag::tagName)) { \
			ostringstream s; \
			const char* buf1 = _sms1->getBinProperty(Tag::tagName, &len1); \
			copy(buf1, buf1 + len1, ostream_iterator<int>(s, ",")); \
			__trace2__(#tagName ": %s != NULL", s.str().c_str()); \
			res.push_back(errCode); \
		} else if (!_sms1->hasBinProperty(Tag::tagName) && _sms2->hasBinProperty(Tag::tagName)) { \
			ostringstream s; \
			const char* buf2 = _sms2->getBinProperty(Tag::tagName, &len2); \
			copy(buf2, buf2 + len2, ostream_iterator<int>(s, ",")); \
			__trace2__(#tagName ": NULL != %s", s.str().c_str()); \
			res.push_back(errCode); \
		} else if (_sms1->hasBinProperty(Tag::tagName) && _sms2->hasBinProperty(Tag::tagName)) { \
			const char* buf1 = _sms1->getBinProperty(Tag::tagName, &len1); \
			const char* buf2 = _sms2->getBinProperty(Tag::tagName, &len2); \
			if (len1 != len2 || memcmp(buf1, buf2, len1)) { \
				ostringstream s1, s2; \
				copy(buf1, buf1 + len1, ostream_iterator<int>(s1, ",")); \
				copy(buf2, buf2 + len2, ostream_iterator<int>(s2, ",")); \
				__trace2__(#tagName ": %s != %s", s1.str().c_str(), s2.str().c_str()); \
				res.push_back(errCode); \
			} \
		} \
	}

vector<int> SmsUtil::compareMessages(const SMS& sms1, const SMS& sms2, uint64_t excludeMask)
{
	SMS* _sms1 = const_cast<SMS*>(&sms1);
	SMS* _sms2 = const_cast<SMS*>(&sms2);
	__require__(_sms1 && _sms2);
	vector<int> res;
	Mask<uint64_t> mask(excludeMask);
	int pos = 0;
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
	__compare_str__(getEServiceType, 12);
	__compare__(isArchivationRequested, 13);
	__compare__(getDeliveryReport, 14);
	__compare__(getBillingRecord, 15);
	__compare__(getOriginatingDescriptor, 16);
	__compare__(getDestinationDescriptor, 17);
	__compare__(getServiceId, 18);
	__compare_str__(getRouteId, 19);
	__compare__(getPriority, 20);
	__compare_str__(getSourceSmeId, 21);
	__compare_str__(getDestinationSmeId, 22);
	//body
	__compare_int_body_tag__(SMPP_SCHEDULE_DELIVERY_TIME, 101);
	__compare_int_body_tag__(SMPP_REPLACE_IF_PRESENT_FLAG, 102);
	__compare_int_body_tag__(SMPP_ESM_CLASS, 103);
	__compare_int_body_tag__(SMPP_DATA_CODING, 104);
	__compare_int_body_tag__(SMPP_SM_LENGTH, 105);
	__compare_int_body_tag__(SMPP_REGISTRED_DELIVERY, 106);
	__compare_int_body_tag__(SMPP_PROTOCOL_ID, 107);
	__compare_bin_body_tag__(SMPP_SHORT_MESSAGE, 108);
	__compare_int_body_tag__(SMPP_PRIORITY, 109);
	__compare_int_body_tag__(SMPP_USER_MESSAGE_REFERENCE, 110);
	__compare_int_body_tag__(SMPP_USSD_SERVICE_OP, 111);
	__compare_int_body_tag__(SMPP_DEST_ADDR_SUBUNIT, 112);
	__compare_int_body_tag__(SMPP_PAYLOAD_TYPE, 113);
	__compare_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, 114);
	__compare_int_body_tag__(SMPP_MS_MSG_WAIT_FACILITIES, 115);
	__compare_int_body_tag__(SMPP_USER_RESPONSE_CODE, 116);
	__compare_int_body_tag__(SMPP_SAR_MSG_REF_NUM, 117);
	__compare_int_body_tag__(SMPP_LANGUAGE_INDICATOR, 118);
	__compare_int_body_tag__(SMPP_SAR_TOTAL_SEGMENTS, 119);
	__compare_int_body_tag__(SMPP_NUMBER_OF_MESSAGES, 120);
	__compare_bin_body_tag__(SMPP_MESSAGE_PAYLOAD, 121);
	__compare_int_body_tag__(SMPP_DATA_SM, 122);
	__compare_int_body_tag__(SMPP_MS_VALIDITY, 123);
	__compare_int_body_tag__(SMPP_MSG_STATE, 124);
	__compare_int_body_tag__(SMSC_DISCHARGE_TIME, 125);
	__compare_str_body_tag__(SMSC_RECIPIENTADDRESS, 126);
	__compare_int_body_tag__(SMSC_STATUS_REPORT_REQUEST, 127);
	__compare_int_body_tag__(SMSC_USSD_OP, 128);
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
	if (mask[pos++]) { \
		/*__trace2__("set_int_body_tag: " #tagName ", pos = %d", pos);*/ \
		uint32_t tmp = value; \
		sms->setIntProperty(Tag::tagName, tmp); \
		if (check) { \
			intMap[Tag::tagName] = tmp; \
		} \
	}

#define __set_str_body_tag__(tagName, length) \
	if (mask[pos++]) { \
		int len = length; \
		/*__trace2__("set_str_body_tag: " #tagName ", pos = %d, length = %d", pos, len);*/ \
		char* data = new char[len + 1]; \
		rand_char(len, data); \
		sms->setStrProperty(Tag::tagName, data); \
		if (check) { \
			strMap.insert(StrMap::value_type(Tag::tagName, CheckData(len, data))); \
		} else { delete data; } \
	}

#define __set_bin_body_tag__(tagName, length) \
	if (mask[pos++]) { \
		int len = length; \
		/*__trace2__("set_bin_body_tag: " #tagName ", pos = %d, length = %d", pos, len);*/ \
		char* data = new char[len + 1]; \
		rand_uint8_t(len, (uint8_t*) data); \
		sms->setBinProperty(Tag::tagName, data, len); \
		if (check) { \
			binMap.insert(BinMap::value_type(Tag::tagName, CheckData(len, data))); \
		} else { delete data; } \
	}

#define __check_int_body_tag__(tagName) \
	IntMap::const_iterator it_##tagName = intMap.find(Tag::tagName); \
	if (it_##tagName == intMap.end()) { \
		__require__(!sms->hasIntProperty(Tag::tagName)); \
	} else { \
		__require__(sms->hasIntProperty(Tag::tagName) && \
			sms->getIntProperty(Tag::tagName) == it_##tagName->second); \
	}
	
#define __check_str_body_tag__(tagName) \
	StrMap::const_iterator it_##tagName = strMap.find(Tag::tagName); \
	if (it_##tagName == strMap.end()) { \
		__require__(!sms->hasStrProperty(Tag::tagName)); \
	} else { \
		pair<int, char*> res = it_##tagName->second; \
		__require__(sms->hasStrProperty(Tag::tagName) && \
			sms->getStrProperty(Tag::tagName) == res.second); \
	}
	
#define __check_bin_body_tag__(tagName) \
	BinMap::const_iterator it_##tagName = binMap.find(Tag::tagName); \
	if (it_##tagName == binMap.end()) { \
		__require__(!sms->hasBinProperty(Tag::tagName)); \
	} else { \
		__require__(sms->hasBinProperty(Tag::tagName)); \
		unsigned len; \
		const char* buf = sms->getBinProperty(Tag::tagName, &len); \
		pair<int, char*> res = it_##tagName->second; \
		__require__(len == res.first && !memcmp(buf, res.second, len)); \
	}
	
void SmsUtil::setupRandomCorrectSms(SMS* sms, uint64_t includeMask, bool check)
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
	__set_int__(uint32_t, ServiceId, rand0(INT_MAX));
	__set_str2__(RouteId, MAX_ROUTE_ID_TYPE_LENGTH);
	__set_int__(int32_t, Priority, rand0(INT_MAX));
	__set_str2__(SourceSmeId, MAX_SMESYSID_TYPE_LENGTH);
	__set_str2__(DestinationSmeId, MAX_SMESYSID_TYPE_LENGTH);
	//поля сохраняются в body случайным образом
	//даже обязательные для sms поля могут не сохраняться в БД
	auto_ptr<uint8_t> tmp = rand_uint8_t(8);
	uint64_t randomMask = *((uint64_t*) tmp.get());
	Mask<uint64_t> mask(includeMask & randomMask);
	int pos = 0;
	__trace2__("mask = %s", mask.str());

	typedef map<int, uint32_t> IntMap;
	typedef pair<int, char*> CheckData;
	typedef map<int, const CheckData> StrMap;
	typedef map<int, const CheckData> BinMap;
	IntMap intMap;
	StrMap strMap;
	BinMap binMap;

	//set fields
	int msgLen = rand1(MAX_SM_LENGTH);
	__set_int_body_tag__(SMPP_SCHEDULE_DELIVERY_TIME, time(NULL) + rand0(24 * 3600));
	__set_int_body_tag__(SMPP_REPLACE_IF_PRESENT_FLAG, rand0(2));
	__set_int_body_tag__(SMPP_ESM_CLASS, rand0(255));
	__set_int_body_tag__(SMPP_DATA_CODING, rand0(255));
	__set_int_body_tag__(SMPP_SM_LENGTH, msgLen);
	__set_int_body_tag__(SMPP_REGISTRED_DELIVERY, rand0(255));
	__set_int_body_tag__(SMPP_PROTOCOL_ID, rand0(255));
	__set_bin_body_tag__(SMPP_SHORT_MESSAGE, msgLen);
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
	__set_bin_body_tag__(SMPP_MESSAGE_PAYLOAD, rand1(MAX_PAYLOAD_LENGTH));
	__set_int_body_tag__(SMPP_DATA_SM, rand0(255));
	__set_int_body_tag__(SMPP_MS_VALIDITY, rand0(255));
	__set_int_body_tag__(SMPP_MSG_STATE, rand0(255));
	__set_int_body_tag__(SMSC_DISCHARGE_TIME, rand0(INT_MAX));
	__set_str_body_tag__(SMSC_RECIPIENTADDRESS, rand1(30));
	__set_int_body_tag__(SMSC_STATUS_REPORT_REQUEST, rand0(255));
	__set_int_body_tag__(SMSC_USSD_OP, rand0(255));
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
		__check_bin_body_tag__(SMPP_SHORT_MESSAGE);
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
		__check_bin_body_tag__(SMPP_MESSAGE_PAYLOAD);
		__check_int_body_tag__(SMPP_DATA_SM);
		__check_int_body_tag__(SMPP_MS_VALIDITY);
		__check_int_body_tag__(SMPP_MSG_STATE);
		__check_int_body_tag__(SMSC_DISCHARGE_TIME);
		__check_str_body_tag__(SMSC_RECIPIENTADDRESS);
		__check_int_body_tag__(SMSC_STATUS_REPORT_REQUEST);
        __check_int_body_tag__(SMSC_USSD_OP);
	}
	//bool attach;
	for (BinMap::iterator it = binMap.begin(); it != binMap.end(); it++)
	{
		const CheckData data = it->second;
		delete data.second;
	}
	for (StrMap::iterator it = strMap.begin(); it != strMap.end(); it++)
	{
		const CheckData data = it->second;
		delete data.second;
	}
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
	sms->setServiceId(0);
	sms->setRouteId("");
	sms->setPriority(0);
	sms->setSourceSmeId("");
	sms->setDestinationSmeId("");
	sms->getMessageBody() = Body();
	//bool attach;
}

const string SmsUtil::configString(const Address& addr)
{
	ostringstream os;
	AddressValue addrVal;
	addr.getValue(addrVal);
	if (addr.getTypeOfNumber() == 1 && addr.getNumberingPlan() == 1)
	{
		os << "+" << addrVal;
	}
	else if (addr.getTypeOfNumber() == 0 && addr.getNumberingPlan() == 1)
	{
		os << addrVal;
	}
	else
	{
		os << "." << (int) addr.getTypeOfNumber() << "." <<
			(int) addr.getNumberingPlan() << "." << addrVal;
	}
	return os.str();
}

bool ltAddress::operator() (const Address& a1, const Address& a2) const
{
	return a1 < a2;
}

ostream& operator<< (ostream& os, const Address& a)
{
	AddressValue addrVal;
	int addrLen = a.getValue(addrVal);
	os << "." << (int) a.getTypeOfNumber() << "." <<
		(int) a.getNumberingPlan() << "." << addrVal;
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

const string str(const Address& addr)
{
	ostringstream os;
	os << addr;
	return os.str();
}

ostream& operator<< (ostream& os, const Descriptor& d)
{
	os << "{msc=" << d.msc << "(" << (int) d.mscLength <<
		"), imsi=" << d.imsi << "(" << (int) d.imsiLength <<
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

#define __print__(field) \
	os << endl << #field << " = " << sms.get##field()

#define __print_int__(field) \
	os << endl << #field << " = " << (int) sms.get##field()

#define __print_bool__(field) \
	os << endl << #field << " = " << (sms.is##field() ? "true" : "false")

#define __print_str__(field) \
	os << endl << #field << " = \"" << sms.get##field() << "\""
	
#define __print_int_body_tag__(tagName) \
	if (sms.hasIntProperty(Tag::tagName)) { \
		os << endl << #tagName << " = " << sms.getIntProperty(Tag::tagName); \
	}

#define __print_str_body_tag__(tagName) \
	if (sms.hasStrProperty(Tag::tagName)) { \
		os << endl << #tagName << " = \"" << sms.getStrProperty(Tag::tagName) << "\""; \
	}

#define __print_bin_body_tag__(tagName) \
	if (sms.hasBinProperty(Tag::tagName)) { \
		ostringstream s; \
		unsigned len; \
		const char* buf = sms.getBinProperty(Tag::tagName, &len); \
		copy(buf, buf + len, ostream_iterator<int>(s, ",")); \
		os << endl << #tagName << " = \"" << s.str() << "\""; \
	}

ostream& operator<< (ostream& os, SMS& sms)
{
	__print_int__(State);
	__print_int__(SubmitTime);
	__print_int__(ValidTime);
	__print_int__(AttemptsCount);
	__print_int__(LastResult);
	__print_int__(LastTime);
	__print_int__(NextTime);
	__print__(OriginatingAddress);
	__print__(DestinationAddress);
	__print__(DealiasedDestinationAddress);
	__print_int__(MessageReference);
	__print_str__(EServiceType);
	__print_bool__(ArchivationRequested);
	__print_int__(DeliveryReport);
	__print_int__(BillingRecord);
	__print__(OriginatingDescriptor);
	__print__(DestinationDescriptor);
	__print_int__(ServiceId);
	__print_str__(RouteId);
	__print_int__(Priority);
	__print_str__(SourceSmeId);
	__print_str__(DestinationSmeId);
	//body
	__print_int_body_tag__(SMPP_SCHEDULE_DELIVERY_TIME);
	__print_int_body_tag__(SMPP_REPLACE_IF_PRESENT_FLAG);
	__print_int_body_tag__(SMPP_ESM_CLASS);
	__print_int_body_tag__(SMPP_DATA_CODING);
	__print_int_body_tag__(SMPP_SM_LENGTH);
	__print_int_body_tag__(SMPP_REGISTRED_DELIVERY);
	__print_int_body_tag__(SMPP_PROTOCOL_ID);
	__print_bin_body_tag__(SMPP_SHORT_MESSAGE);
	__print_int_body_tag__(SMPP_PRIORITY);
	__print_int_body_tag__(SMPP_USER_MESSAGE_REFERENCE);
	__print_int_body_tag__(SMPP_USSD_SERVICE_OP);
	__print_int_body_tag__(SMPP_DEST_ADDR_SUBUNIT);
	__print_int_body_tag__(SMPP_PAYLOAD_TYPE);
	__print_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID);
	__print_int_body_tag__(SMPP_MS_MSG_WAIT_FACILITIES);
	__print_int_body_tag__(SMPP_USER_RESPONSE_CODE);
	__print_int_body_tag__(SMPP_SAR_MSG_REF_NUM);
	__print_int_body_tag__(SMPP_LANGUAGE_INDICATOR);
	__print_int_body_tag__(SMPP_SAR_TOTAL_SEGMENTS);
	__print_int_body_tag__(SMPP_NUMBER_OF_MESSAGES);
	__print_bin_body_tag__(SMPP_MESSAGE_PAYLOAD);
    __print_int_body_tag__(SMPP_DATA_SM);
	__print_int_body_tag__(SMPP_MS_VALIDITY);
	__print_int_body_tag__(SMPP_MSG_STATE);
	__print_int_body_tag__(SMSC_DISCHARGE_TIME);
	__print_str_body_tag__(SMSC_RECIPIENTADDRESS);
	__print_int_body_tag__(SMSC_STATUS_REPORT_REQUEST);
    __print_int_body_tag__(SMSC_USSD_OP);

	//bool attach;
}

const string str(SMS& sms)
{
	ostringstream os;
	os << sms;
	return os.str();
}

}
}
}

