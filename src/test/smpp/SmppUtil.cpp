#include "SmppUtil.hpp"
#include "test/util/Util.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "sms/sms.h"
#include "util/Exception.hpp"
#include "util/debug.h"
#include <algorithm>
#include <ctime>
#include <map>

namespace smsc {
namespace test {
namespace smpp {

using smsc::util::Exception;
using smsc::test::conf::TestConfig;
using smsc::test::util::operator<<;
using namespace std;
using namespace smsc::sms; //constants
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;
//using smsc::test::sms::SmsUtil;

static const int MAX_OSTR_PRINT_SIZE = 255;

void dumpPdu(const char* tc, const string& id, SmppHeader* pdu)
{
	if (pdu)
	{
		ostringstream os;
		os << tc << "(): systemId = " << id <<
			", sequenceNumber = " << pdu->get_sequenceNumber();
		switch (pdu->get_commandId())
		{
			case SUBMIT_SM:
				{
					PduSubmitSm* p = reinterpret_cast<PduSubmitSm*>(pdu);
					os << ", scheduleDeliveryTime = " << SmppUtil::getWaitTime(p->get_message().get_scheduleDeliveryTime(), time(NULL));
					os << ", validityPeriod = " << SmppUtil::getValidTime(p->get_message().get_validityPeriod(), time(NULL));
				}
				break;
			case REPLACE_SM:
				{
					PduReplaceSm* p = reinterpret_cast<PduReplaceSm*>(pdu);
					os << ", scheduleDeliveryTime = " << SmppUtil::getWaitTime(p->get_scheduleDeliveryTime(), time(NULL));
					os << ", validityPeriod = " << SmppUtil::getValidTime(p->get_validityPeriod(), time(NULL));
				}
				break;
		}
		time_t lt = time(NULL);
		tm t;
		char buf[30];
		os << ", system time = " << asctime_r(localtime_r(&lt, &t), buf) <<
			", pdu:" << endl << *pdu;
	}
	else
	{
		__trace2__("%s(): pdu = NULL", tc);
	}
}

PduAddress* SmppUtil::convert(const Address& smsAddr, PduAddress* smppAddr)
{
	smppAddr->set_typeOfNumber(smsAddr.getTypeOfNumber());
	smppAddr->set_numberingPlan(smsAddr.getNumberingPlan());
	AddressValue val;
	uint8_t len = smsAddr.getValue(val);
	smppAddr->set_value(val);
	return smppAddr;
}

Address* SmppUtil::convert(PduAddress& smppAddr, Address* smsAddr)
{
	smsAddr->setTypeOfNumber(smppAddr.get_typeOfNumber());
	smsAddr->setNumberingPlan(smppAddr.get_numberingPlan());
	const char* val = smppAddr.get_value();
	smsAddr->setValue(strlen(val), val);
	return smsAddr;
}

/*
SMSId SmppUtil::convert(const char* id)
{
    return atol(id);
}

MessageId& SmppUtil::convert(const SMSId& smsId, MessageId& smppId)
{
	sprintf(smppId, "%d", smsId);
	return smppId;
}
*/

const char* SmppUtil::time2string(time_t lt, char* str, time_t base, int num, bool check)
{
	switch (num)
	{
		case 1: //GMT time format
			{
				tm t;
				int len = strftime(str, MAX_SMPP_TIME_LENGTH + 1,
					"%y%m%d%H%M%S000+", gmtime_r(&lt, &t));
			}
			break;
		case 2: //local time format (+)
			{
				tm t;
				int len = strftime(str, MAX_SMPP_TIME_LENGTH + 1,
					"%y%m%d%H%M%S0", localtime_r(&lt, &t));
				int qtz = -timezone / 900 + (t.tm_isdst ? 4 : 0);
				//для Новосибирка export TZ=NSD-6, а timezone = -21600
				sprintf(str + len, "%02d%c", abs(qtz), qtz < 0 ? '-' : '+');
			}
			break;
		case 3: //Relative time format
			{
				time_t df = lt - base;
				__require__(df >= 0);
				tm t;
				t.tm_year = (df / 31104000);
				t.tm_mon = (df / 2592000) % 12;
				t.tm_mday = (df / 86400) % 30;
				t.tm_hour = (df / 3600) % 24;
				t.tm_min = (df / 60) % 60;
				t.tm_sec = df % 60;
				//t.tm_wday; t.tm_yday; t.tm_isdst;
				sprintf(str, "%02d%02d%02d%02d%02d%02d000R",
					t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
			}
			break;
		default:
			__unreachable__("Invalid num");
	}
	if (check)
	{
		__require__(string2time(str, base, false) == lt);
	}
	return str;
}

time_t SmppUtil::string2time(const char* str, time_t base, bool check)
{
	if (strlen(str) != MAX_SMPP_TIME_LENGTH)
	{
		return 0;
	}
	int tz;
	char p;
	tm t;
	t.tm_isdst = 0;
	int tenthSec;
	int res = sscanf(str, "%2d%2d%2d%2d%2d%2d%1d%2d%c", &t.tm_year, &t.tm_mon,
		&t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &tenthSec, &tz, &p);
	if (res != 9)
	{
		return 0;
	}
	time_t lt;
	char buf[MAX_SMPP_TIME_LENGTH];
	switch (p)
	{
		case '+':
			t.tm_year += 100;
			t.tm_mon -= 1;
			lt = mktime(&t) - timezone - tz * 900;
			if (check)
			{
				__require__(!strcmp(time2string(lt, buf, base, tz ? 2 : 1, false), str));
			}
			return lt;
		case '-':
			t.tm_year += 100;
			t.tm_mon -= 1;
			lt = mktime(&t) - timezone + tz * 900;
			if (check)
			{
				__require__(!strcmp(time2string(lt, buf, base, tz ? 2 : 1, false), str));
			}
			return lt;
		case 'R':
			lt = base + t.tm_year * 31104000 + t.tm_mon * 2592000 +
				t.tm_mday * 86400 + t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
			if (check)
			{
				__require__(!strcmp(time2string(lt, buf, base, 3, false), str));
			}
			return lt;
		default:
			return 0;
	}
}

time_t SmppUtil::getWaitTime(const char* str, time_t submitTime)
{
	//если schedule_delivery_time = NULL, то немедленная доставка
	time_t waitTime = str && strlen(str) ?
		SmppUtil::string2time(str, submitTime) : submitTime;
	return waitTime;
}

time_t SmppUtil::getValidTime(const char* str, time_t submitTime)
{
	//если validity_period = NULL, то срок валидности по умолчанию
	__cfg_int__(maxValidPeriod);
	time_t validTime = str && strlen(str) ?
		SmppUtil::string2time(str, submitTime) : submitTime + maxValidPeriod;
	//если validity_period > maxValidPeriod, то maxValidPeriod
	validTime = min(validTime, submitTime + maxValidPeriod);
	return validTime;
}

/*
bool SmppUtil::compareAddresses(PduAddress& a1, PduAddress& a2)
{
	return (a1.get_typeOfNumber() == a2.get_typeOfNumber() &&
		a1.get_numberingPlan() == a2.get_numberingPlan() &&
		strcmp(a1.get_value(), a2.get_value()) == 0);
}
*/

#define __trace_compare__ \
	/*__trace2__("[%d]: " #field, pos - 1);*/
	
#define __compare__(field, expr, errCode) \
	if (!mask[pos++] && ((p1.has_##field() && !p2.has_##field()) || \
		(!p1.has_##field() && p2.has_##field()) || \
		(p1.has_##field() && p2.has_##field() && (expr)))) { \
		__trace_compare__; \
		res.push_back(errCode); }

#define __compare_int__(field, errCode) \
	__compare__(field, p1.get_##field() != p2.get_##field(), errCode)

#define __compare_intarr__(field, errCode) \
	__compare__(field, sizeof(p1.get_##field()) != sizeof(p2.get_##field()) || \
		memcmp(p1.get_##field(), p2.get_##field(), sizeof(p1.get_##field())), errCode)

#define __compare_cstr__(field, errCode) \
	__compare__(field, strcmp(p1.get_##field(), p2.get_##field()), errCode)

#define __compare_ostr__(field, errCode) \
	__compare__(field, p1.size_##field() != p2.size_##field() || \
		strncmp(p1.get_##field(), p2.get_##field(), p1.size_##field()), errCode)

vector<int> SmppUtil::compareOptional(SmppOptional& p1, SmppOptional& p2,
	uint64_t excludeMask)
{
	vector<int> res;
	Mask<uint64_t> mask(excludeMask);
	int pos = 0;
	__compare_int__(destAddrSubunit, 1);
	__compare_int__(sourceAddrSubunit, 2);
	__compare_int__(destNetworkType, 3);
	__compare_int__(sourceNetworkType, 4);
	__compare_int__(destBearerType, 5);
	__compare_int__(sourceBearerType, 6);
	__compare_int__(destTelematicsId, 7);
	__compare_int__(sourceTelematicsId, 8);
	__compare_int__(qosTimeToLive, 9);
	__compare_int__(payloadType, 10);
	__compare_cstr__(additionalStatusInfoText, 11);
	__compare_cstr__(receiptedMessageId, 12);
	__compare_int__(msMsgWaitFacilities, 13);
	__compare_int__(privacyIndicator, 14);
	__compare_ostr__(sourceSubaddress, 15);
	__compare_ostr__(destSubaddress, 16);
	__compare_int__(userMessageReference, 17);
	__compare_int__(userResponseCode, 18);
	__compare_int__(languageIndicator, 19);
	__compare_int__(sourcePort, 20);
	__compare_int__(destinationPort, 21);
	__compare_int__(sarMsgRefNum, 22);
	__compare_int__(sarTotalSegments, 23);
	__compare_int__(sarSegmentSegnum, 24);
	__compare_int__(scInterfaceVersion, 25);
	__compare_int__(displayTime, 26);
	__compare_int__(msValidity, 27);
	__compare_int__(dpfResult, 28);
	__compare_int__(setDpf, 29);
	__compare_int__(msAvailableStatus, 30);
	__compare_intarr__(networkErrorCode, 31);
	__compare_ostr__(messagePayload, 32);
	__compare_int__(deliveryFailureReason, 33);
	__compare_int__(moreMessagesToSend, 34);
	__compare_int__(messageState, 35);
	__compare_ostr__(callbackNum, 36);
	__compare_int__(callbackNumPresInd, 37);
	__compare_ostr__(callbackNumAtag, 38);
	__compare_int__(numberOfMessages, 39);
	__compare_int__(smsSignal, 40);
	__compare_int__(alertOnMessageDelivery, 41);
	__compare_int__(itsReplyType, 42);
	__compare_intarr__(itsSessionInfo, 43);
	__compare_int__(ussdServiceOp, 44);
	return res;
}

#define __trace_set__(name) \
	/*__trace__(name ": " #field);*/
	
#define __set_int__(type, field, value) \
	__trace_set__("set_int"); \
	type tmp_##field = value; \
	p.set_##field(tmp_##field); \
	if (check) { __require__(p.get_##field() == tmp_##field); }

#define __set_ostr__(field, length) \
	__trace_set__("set_ostr"); \
	int len_##field = length; \
	auto_ptr<char> str_##field = rand_char(len_##field); \
	p.set_##field(str_##field.get(), len_##field); \
	if (check) { __require__(p.size_##field() == len_##field && \
		!strncmp(p.get_##field(), str_##field.get(), len_##field)); }

#define __set_bin__(field, length, dataCoding) \
	__trace_set__("set_bin"); \
	int len_##field = length; \
	uint8_t dc_##field = dataCoding; \
	auto_ptr<char> str_##field = rand_text(len_##field, dc_##field); \
	p.set_##field(str_##field.get(), len_##field); \
	if (check) { __require__(p.size_##field() == len_##field && \
		!memcmp(p.get_##field(), str_##field.get(), len_##field)); }

#define __set_cstr__(field, length) \
	__trace_set__("set_cstr"); \
	auto_ptr<char> str_##field = rand_char(length); \
	if (check) { p.set_##field(str_##field.get()); \
	__require__(!strcmp(p.get_##field(), str_##field.get())); }

#define __set_cstr2__(field, value) \
	__trace_set__("set_cstr2"); \
	const char* val_##field = value; \
	p.set_##field(val_##field); \
	if (check) { __require__(!strcmp(p.get_##field(), val_##field)); }
	
#define __set_addr__(field) \
	__trace_set__("set_addr"); \
	PduAddress tmp_##field; \
	setupRandomCorrectAddress(&tmp_##field); \
	p.set_##field(tmp_##field); \
	if (check) { __require__(p.get_##field() == tmp_##field); }

void SmppUtil::setupRandomCorrectAddress(PduAddress* addr, bool check)
{
	__require__(addr);
	PduAddress& p = *addr;
	__set_int__(uint8_t, typeOfNumber, rand0(255));
	__set_int__(uint8_t, numberingPlan, rand0(255));
	__set_cstr__(value, rand1(MAX_ADDRESS_VALUE_LENGTH));
}

void SmppUtil::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu,
	uint64_t mask, bool check)
{
	__require__(pdu);
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(sequentialPduInterval);
	PduPartSm& p = pdu->get_message();
	SmppTime tmp;
	//set & check fields
	__set_cstr__(serviceType, MAX_ESERVICE_TYPE_LENGTH);
	__set_addr__(source);
	__set_addr__(dest);
	__set_int__(uint8_t, esmClass, rand0(255));
	__set_int__(uint8_t, protocolId, rand0(255));
	__set_int__(uint8_t, priorityFlag, rand0(255));
	time_t waitTime = time(NULL) + rand0(maxWaitTime);
	time_t validTime = waitTime + rand0(maxDeliveryPeriod);
	__set_cstr2__(scheduleDeliveryTime, time2string(waitTime, tmp, time(NULL), __numTime__));
	__set_cstr2__(validityPeriod, time2string(validTime, tmp, time(NULL), __numTime__));
	__set_int__(uint8_t, registredDelivery, rand0(255));
	__set_int__(uint8_t, replaceIfPresentFlag, !rand0(10));
	uint8_t dataCoding = getDataCoding(RAND_TC);
	__set_int__(uint8_t, dataCoding, dataCoding);
	__set_int__(uint8_t, smDefaultMsgId, rand0(255)); //хбз что это такое
	__set_bin__(shortMessage, rand1(MAX_SHORT_MESSAGE_LENGTH), dataCoding);
	mask &= ~OPT_USER_MSG_REF; //исключить userMessageReference
	setupRandomCorrectOptionalParams(pdu->get_optional(), mask, check);
}

void SmppUtil::setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu,
	uint8_t dataCoding, uint64_t mask, bool check)
{
	__require__(pdu);
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(sequentialPduInterval);
	PduReplaceSm& p = *pdu;
	SmppTime tmp;
	//set & check fields
	__set_cstr__(messageId, MAX_MSG_ID_LENGTH);
	__set_addr__(source);
	time_t waitTime = time(NULL) + rand0(maxWaitTime);
	time_t validTime = waitTime + rand0(maxDeliveryPeriod);
	__set_cstr2__(scheduleDeliveryTime, time2string(waitTime, tmp, time(NULL), __numTime__));
	__set_cstr2__(validityPeriod, time2string(validTime, tmp, time(NULL), __numTime__));
	__set_int__(uint8_t, registredDelivery, rand0(255));
	__set_int__(uint8_t, smDefaultMsgId, rand0(255)); //хбз что это такое
	__set_bin__(shortMessage, rand1(MAX_SHORT_MESSAGE_LENGTH), dataCoding);
}

#define __trace_set_optional__(name, field) \
	__trace2__(name "[%d]: " #field, pos - 1);
	
#define __set_optional_int__(type, field, value) \
	if (mask[pos++]) { \
		__trace_set_optional__("set_optional_int", field); \
		type tmp_##field = value; \
		opt.set_##field(tmp_##field); \
		if (check) { intMap.insert(IntMap::value_type(#field, tmp_##field)); } \
	}

#define __set_optional_intarr__(field, value, length) \
	if (mask[pos++]) { \
		__require__(length <= 4); \
		__trace_set_optional__("set_optional_intarr", field); \
		uint8_t* tmp_##field = value; \
		opt.set_##field(tmp_##field); \
		if (check) { \
			uint32_t int_##field; \
			memcpy(&int_##field, tmp_##field, length); \
			intMap.insert(IntMap::value_type(#field, int_##field)); \
		} \
	}

#define __set_optional_ostr__(field, length) \
	if (mask[pos++]) { \
		__trace_set_optional__("set_optional_ostr", field); \
		int len_##field = length; \
		auto_ptr<char> str_##field = rand_char(len_##field); \
		opt.set_##field(str_##field.get(), len_##field); \
		if (check) { \
			OStr tmp_##field; \
			tmp_##field.copy(str_##field.get(), len_##field); \
			ostrMap.insert(OStrMap::value_type(#field, tmp_##field)); \
		} \
	}

#define __set_optional_bin__(field, length, dataCoding) \
	if (mask[pos++]) { \
		__trace_set_optional__("set_optional_bin", field); \
		int len_##field = length; \
		uint8_t dc_##field = dataCoding; \
		auto_ptr<char> str_##field = rand_text(len_##field, dc_##field); \
		opt.set_##field(str_##field.get(), len_##field); \
		if (check) { \
			OStr tmp_##field; \
			tmp_##field.copy((char*) str_##field.get(), len_##field); \
			ostrMap.insert(OStrMap::value_type(#field, tmp_##field)); \
		} \
	}

#define __set_optional_cstr__(field, length) \
	if (mask[pos++]) { \
		__trace_set_optional__("set_optional_cstr", field); \
		auto_ptr<char> str_##field = rand_char(length); \
		opt.set_##field(str_##field.get()); \
		if (check) { \
			COStr tmp_##field; \
			tmp_##field.copy(str_##field.get()); \
			cstrMap.insert(CStrMap::value_type(#field, tmp_##field)); \
		} \
	}

#define __skip__ \
	pos++;

#define __check_optional_int__(field) \
	IntMap::const_iterator it_##field = intMap.find(#field); \
	if (it_##field == intMap.end()) { \
		__require__(!opt.has_##field()); \
	} else { \
		__require__(opt.has_##field() && \
			opt.get_##field() == it_##field->second); \
	}
	
#define __check_optional_intarr__(field) \
	IntMap::const_iterator it_##field = intMap.find(#field); \
	if (it_##field == intMap.end()) { \
		__require__(!opt.has_##field()); \
	} else { \
		__require__(opt.has_##field() && \
			memcmp(opt.get_##field(), &it_##field->second, sizeof(opt.get_##field()))); \
	}
	
#define __check_optional_ostr__(field) \
	OStrMap::iterator it_##field = ostrMap.find(#field); \
	if (it_##field == ostrMap.end()) { \
		__require__(!opt.has_##field()); \
	} else { \
		__require__(opt.has_##field() && \
			opt.size_##field() == it_##field->second.size() && \
			!memcmp(opt.get_##field(), it_##field->second.cstr(), opt.size_##field())); \
	}

#define __check_optional_cstr__(field) \
	CStrMap::iterator it_##field = cstrMap.find(#field); \
	if (it_##field == cstrMap.end()) { \
		__require__(!opt.has_##field()); \
	} else { \
		__require__(opt.has_##field() && \
			!strcmp(opt.get_##field(), it_##field->second.cstr())); \
	}

void SmppUtil::setupRandomCorrectOptionalParams(SmppOptional& opt,
	uint8_t dataCoding, uint64_t _mask, bool check)
{
	//поля сохраняются случайным образом
	auto_ptr<uint8_t> tmp = rand_uint8_t(8);
	Mask<uint64_t> mask(_mask & *((uint64_t*) tmp.get()));
	int pos = 0;
	//__trace2__("SmppUtil::setupRandomCorrectOptionalParams(): mask = %s", mask.str());

	typedef map<const string, uint32_t> IntMap;
	typedef map<const string, COStr> CStrMap;
	typedef map<const string, OStr> OStrMap;
	CStrMap cstrMap;
	OStrMap ostrMap;
	IntMap intMap;

	//set fields
	__set_optional_int__(uint8_t, destAddrSubunit, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, sourceAddrSubunit, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, destNetworkType, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, sourceNetworkType, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, destBearerType, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, sourceBearerType, rand0(255));
	__skip__; //__set_optional_int__(uint16_t, destTelematicsId, rand0(65535));
	__skip__; //__set_optional_int__(uint8_t, sourceTelematicsId, rand0(255));
	__skip__; //__set_optional_int__(uint32_t, qosTimeToLive, rand0(INT_MAX));
	__set_optional_int__(uint8_t, payloadType, rand0(255));
	__skip__; //__set_optional_cstr__(additionalStatusInfoText, rand1(255));
	__set_optional_cstr__(receiptedMessageId, rand1(MAX_MSG_ID_LENGTH));
	__set_optional_int__(uint8_t, msMsgWaitFacilities, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, privacyIndicator, rand0(255));
	__skip__; //__set_optional_ostr__(sourceSubaddress, rand2(2, 22));
	__skip__; //__set_optional_ostr__(destSubaddress, rand2(2, 22));
	__set_optional_int__(uint16_t, userMessageReference, rand0(65535));
	__set_optional_int__(uint8_t, userResponseCode, rand0(255));
	__set_optional_int__(uint8_t, languageIndicator, rand0(255));
	__skip__; //__set_optional_int__(uint16_t, sourcePort, rand0(65535));
	__skip__; //__set_optional_int__(uint16_t, destinationPort, rand0(65535));
	__set_optional_int__(uint16_t, sarMsgRefNum, rand0(65535));
	__set_optional_int__(uint8_t, sarTotalSegments, rand1(255));
	__skip__; //__set_optional_int__(uint8_t, sarSegmentSegnum, rand1(255));
	__skip__; //__set_optional_int__(uint8_t, scInterfaceVersion, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, displayTime, rand0(255));
	__set_optional_int__(uint8_t, msValidity, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, dpfResult, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, setDpf, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, msAvailableStatus, rand0(255));
	//int errCode = rand0(INT_MAX);
	__skip__; //__set_optional_intarr__(networkErrorCode, (uint8_t*) &errCode, 3);
	__set_optional_bin__(messagePayload, rand0(65535), dataCoding);
	__skip__; //__set_optional_int__(uint8_t, deliveryFailureReason, rand0(255));
	__skip__; //__set_optional_int__(uint8_t, moreMessagesToSend, rand0(255));
	__set_optional_int__(uint8_t, messageState, rand0(255));
	__skip__; //__set_optional_ostr__(callbackNum, rand2(4, 19));
	__skip__; //__set_optional_int__(uint8_t, callbackNumPresInd, rand0(255));
	__skip__; //__set_optional_ostr__(callbackNumAtag, rand0(65));
	__set_optional_int__(uint8_t, numberOfMessages, rand0(255));
	__skip__; //__set_optional_int__(uint16_t, smsSignal, rand0(65535));
	__skip__; //__set_optional_int__(bool, alertOnMessageDelivery, rand0(1));
	__skip__; //__set_optional_int__(uint8_t, itsReplyType, rand0(255));
	//int sessInfo = rand0(INT_MAX);
	__skip__; //__set_optional_intarr__(itsSessionInfo, (uint8_t*) &sessInfo, 3);
	__set_optional_int__(uint8_t, ussdServiceOp, rand0(255));
	
	//check fields
	__check_optional_int__(destAddrSubunit);
	//__check_optional_int__(sourceAddrSubunit);
	//__check_optional_int__(destNetworkType);
	//__check_optional_int__(sourceNetworkType);
	//__check_optional_int__(destBearerType);
	//__check_optional_int__(sourceBearerType);
	//__check_optional_int__(destTelematicsId);
	//__check_optional_int__(sourceTelematicsId);
	//__check_optional_int__(qosTimeToLive);
	__check_optional_int__(payloadType);
	//__check_optional_cstr__(additionalStatusInfoText);
	__check_optional_cstr__(receiptedMessageId);
	__check_optional_int__(msMsgWaitFacilities);
	//__check_optional_int__(privacyIndicator);
	//__check_optional_ostr__(sourceSubaddress);
	//__check_optional_ostr__(destSubaddress);
	__check_optional_int__(userMessageReference);
	__check_optional_int__(userResponseCode);
	__check_optional_int__(languageIndicator);
	//__check_optional_int__(sourcePort);
	//__check_optional_int__(destinationPort);
	__check_optional_int__(sarMsgRefNum);
	__check_optional_int__(sarTotalSegments);
	//__check_optional_int__(sarSegmentSegnum);
	//__check_optional_int__(scInterfaceVersion);
	//__check_optional_int__(displayTime);
	__check_optional_int__(msValidity);
	//__check_optional_int__(dpfResult);
	//__check_optional_int__(setDpf);
	//__check_optional_int__(msAvailableStatus);
	//__check_optional_intarr__(networkErrorCode);
	__check_optional_ostr__(messagePayload);
	//__check_optional_int__(deliveryFailureReason);
	//__check_optional_int__(moreMessagesToSend);
	__check_optional_int__(messageState);
	//__check_optional_ostr__(callbackNum);
	//__check_optional_int__(callbackNumPresInd);
	//__check_optional_ostr__(callbackNumAtag);
	__check_optional_int__(numberOfMessages);
	//__check_optional_int__(smsSignal);
	//__check_optional_int__(alertOnMessageDelivery);
	//__check_optional_int__(itsReplyType);
	//__check_optional_intarr__(itsSessionInfo);
	__check_optional_int__(ussdServiceOp);
}

/*
void SmppUtil::setupRandomCorrectSubmitMultiPdu(PduMultiSm* pdu)
{
	//PduPartSm
	__set__(serviceType(c_str);
	__set__(source(PduAddress);
	__set__(numberOfDests(uint8_t);
	__set__(dests(PduDestAddress);
	__set__(esmClass(uint8_t);
	__set__(protocolId(uint8_t);
	__set__(priorityFlag(uint8_t);
	__set__(scheduleDeliveryTime(c_str);
	__set__(validityPeriod(c_str);
	__set__(registredDelivery(uint8_t);
	__set__(replaceIfPresentFlag(uint8_t);
	__set__(dataCoding(uint8_t);
	__set__(smDefaultMsgId(uint8_t);
	__set__(shortMessage(const char* __value,int __len) ;
	//SmppOptional
}
*/

SmppHeader* SmppUtil::copyPdu(SmppHeader* pdu)
{
	__require__(pdu);
	int sz = calcSmppPacketLength(pdu);
	char buf[sz];
	SmppStream s1;
	assignStreamWith(&s1, buf, sz, false);
	if (!fillSmppPdu(&s1, pdu)) throw Exception("Failed to fill smpp packet");
	SmppStream s2;
	assignStreamWith(&s2, buf, sz, true);
	return fetchSmppPdu(&s2);
}

bool operator==(PduAddress& a1, PduAddress& a2)
{
	return (a1.get_typeOfNumber() == a2.get_typeOfNumber() &&
		a1.get_numberingPlan() == a2.get_numberingPlan() &&
		strcmp(a1.get_value(), a2.get_value()) == 0);
}

bool operator!=(PduAddress& a1, PduAddress& a2)
{
	return !operator==(a1, a2);
}

#define __prop__(prop) \
	os << "    " #prop " = " << p.get_##prop() << endl;

#define __str_prop__(prop) \
	os << "    " #prop " = " << (p.get_##prop() ? p.get_##prop() : "NULL") << endl; \
	
#define __ostr_prop__(prop) \
	os << "    " #prop ": length = " << p.size_##prop() << endl; \
	os << hex; \
	const unsigned char* val_##prop = (const unsigned char*) p.get_##prop(); \
	for (int i = 0; i < p.size_##prop() && i < MAX_OSTR_PRINT_SIZE; i++) { \
		unsigned int ch = (unsigned int) *(val_##prop + i); \
		os << (ch < 0x10 ? "0" : "") << ch << " "; \
	} \
	os << dec; \
	os << (p.size_##prop() < MAX_OSTR_PRINT_SIZE ? "" : "...") << endl;

/*
#define __decoded_ostr_prop__(prop, dataCoding) \
	const string str_##prop = decode(p.get_##prop(), p.size_##prop(), dataCoding);
	os << "    " #prop ": length = " << p.size_##prop() << " ("; \
	if (str_##prop.length() < MAX_OSTR_PRINT_SIZE) { \
		os << str_##prop << ")" << endl; \
	} else { \
		os << string(str_##prop, 0, MAX_OSTR_PRINT_SIZE) << "...)" << endl; \
	} \
*/

ostream& operator<< (ostream& os, PduXSm& p)
{
	os << "PduXSm {" << endl;
	os << p.get_header() << endl;
	os << p.get_message() << endl;
	os << p.get_optional() << endl;
	os << "}";
	return os;
}

ostream& operator<< (ostream& os, PduReplaceSm& p)
{
	os << "PduReplaceSm {" << endl;
	os << p.get_header() << endl;
	__str_prop__(messageId);
	__prop__(source);
	__str_prop__(scheduleDeliveryTime);
	__str_prop__(validityPeriod);
	__prop__(registredDelivery);
	__prop__(smDefaultMsgId);
	__ostr_prop__(shortMessage);
	os << "}";
	return os;
}

ostream& operator<< (ostream& os, PduDataSm& p)
{
	os << "PduDataSm {" << endl;
	os << p.get_header() << endl;
	os << p.get_data() << endl;
	os << p.get_optional() << endl;
	os << "}";
	return os;
}

ostream& operator<< (ostream& os, PduQuerySm& p)
{
	os << "PduQuerySm {" << endl;
	os << p.get_header() << endl;
	__str_prop__(messageId);
	__prop__(source);
	os << "}";
	return os;
}

ostream& operator<< (ostream& os, PduXSmResp& p)
{
	os << "PduXSmResp {" << endl;
	os << p.get_header() << endl;
	__str_prop__(messageId);
	os << "}";
	return os;
}

ostream& operator<< (ostream& os, PduReplaceSmResp& p)
{
	os << "PduReplaceSmResp {" << endl;
	os << p.get_header() << endl;
	os << "}";
	return os;
}

ostream& operator<< (ostream& os, SmppHeader& p)
{
	os << "  SmppHeader {" << endl;
	__prop__(commandLength);
	__prop__(commandId);
	__prop__(commandStatus);
	__prop__(sequenceNumber);
	os << "  }";
	return os;
}

ostream& operator<< (ostream& os, PduAddress& addr)
{
    os << "." << addr.get_typeOfNumber() <<
		"." << addr.get_numberingPlan() <<
		"." << addr.get_value();
	return os;
}

ostream& operator<< (ostream& os, PduPartSm& p)
{
    os << "  PduPartSm {" << endl;
	__str_prop__(serviceType);
	__prop__(source);
	__prop__(dest);
	__prop__(numberOfDests);
	//__ptr_property__(PduDestAddress,dests)
	__prop__(esmClass);
	__prop__(protocolId);
	__prop__(priorityFlag);
	__str_prop__(scheduleDeliveryTime);
	__str_prop__(validityPeriod);
	__prop__(registredDelivery);
	__prop__(replaceIfPresentFlag);
	__prop__(dataCoding);
	__prop__(smDefaultMsgId);
	__ostr_prop__(shortMessage);
	os << "  }";
	return os;
}

ostream& operator<< (ostream& os, PduDataPartSm& p)
{
    os << "  PduPartSm {" << endl;
	__str_prop__(serviceType);
	__prop__(source);
	__prop__(dest);
	__prop__(esmClass);
	__prop__(registredDelivery);
	__prop__(dataCoding);
	os << "  }";
	return os;
}

#define __opt_prop__(prop) \
	if (p.has_##prop()) { \
		__prop__(prop); \
	}

#define __opt_str_prop__(prop) \
	if (p.has_##prop()) { \
		__str_prop__(prop); \
	}

#define __opt_ostr_prop__(prop) \
	if (p.has_##prop()) { \
		__ostr_prop__(prop); \
	}

#define __opt_int_arr_prop__(prop, size) \
	if (p.has_##prop()) { \
		os << "    " #prop " = "; \
		os << hex; \
		for (int i = 0; i < size; i++) { \
			os << (unsigned int) p.get_##prop()[i] << " "; \
		} \
		os << dec << endl; \
	}

ostream& operator<< (ostream& os, SmppOptional& p)
{
	os << "  SmppOptional {" << endl;
	__opt_prop__(destAddrSubunit);
	__opt_prop__(sourceAddrSubunit);
	__opt_prop__(destNetworkType);
	__opt_prop__(sourceNetworkType);
	__opt_prop__(destBearerType);
	__opt_prop__(sourceBearerType);
	__opt_prop__(destTelematicsId);
	__opt_prop__(sourceTelematicsId);
	__opt_prop__(qosTimeToLive);
	__opt_prop__(payloadType);
	__opt_str_prop__(additionalStatusInfoText);
	__opt_str_prop__(receiptedMessageId);
	__opt_prop__(msMsgWaitFacilities);
	__opt_prop__(privacyIndicator);
	__opt_ostr_prop__(sourceSubaddress);
	__opt_ostr_prop__(destSubaddress);
	__opt_prop__(userMessageReference);
	__opt_prop__(userResponseCode);
	__opt_prop__(languageIndicator);
	__opt_prop__(sourcePort);
	__opt_prop__(destinationPort);
	__opt_prop__(sarMsgRefNum);
	__opt_prop__(sarTotalSegments);
	__opt_prop__(sarSegmentSegnum);
	__opt_prop__(scInterfaceVersion);
	__opt_prop__(displayTime);
	__opt_prop__(msValidity);
	__opt_prop__(dpfResult);
	__opt_prop__(setDpf);
	__opt_prop__(msAvailableStatus);
	__opt_int_arr_prop__(networkErrorCode, 3);
	__opt_ostr_prop__(messagePayload);
	__opt_prop__(deliveryFailureReason);
	__opt_prop__(moreMessagesToSend);
	__opt_prop__(messageState);
	__opt_ostr_prop__(callbackNum);
	__opt_prop__(callbackNumPresInd);
	__opt_ostr_prop__(callbackNumAtag);
	__opt_prop__(numberOfMessages);
	__opt_prop__(smsSignal);
	__opt_prop__(itsReplyType);
	__opt_int_arr_prop__(itsSessionInfo, 2);
	__opt_prop__(ussdServiceOp);
	__opt_prop__(alertOnMessageDelivery);
	os << "  }";
	return os;
}

}
}
}

