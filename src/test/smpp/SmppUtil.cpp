#include "SmppUtil.hpp"
#include "test/util/Util.hpp"
#include "sms/sms.h"
#include <ctime>

namespace smsc {
namespace test {
namespace smpp {

using namespace smsc::sms; //constants
using namespace smsc::test::util;
//using smsc::test::sms::SmsUtil;

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

SMSId SmppUtil::convert(const char* id)
{
    return atol(id);
}

MessageId& SmppUtil::convert(const SMSId& smsId, MessageId& smppId)
{
	sprintf(smppId, "%d", smsId);
	return smppId;
}

const char* SmppUtil::time2string(time_t lt, char* str, time_t base, int num)
{
	switch (num)
	{
		case 1: //GMT time format
			{
				int len = strftime(str, MAX_SMPP_TIME_LENGTH,
					"%y%m%d%H%M%S000+", gmtime(&lt));
			}
			break;
		case 2: //local time format (+)
			{
				int len = strftime(str, MAX_SMPP_TIME_LENGTH,
					"%y%m%d%H%M%S0", localtime(&lt));
				int tz = timezone / 900;
				//для Новосибирка export TZ=NSD-6, а timezone = -21600
				sprintf(str + len, "%02d%c", abs(tz), tz < 0 ? '+' : '-');
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
			throw "";
	}
	return str;
}

time_t SmppUtil::string2time(const char* str, time_t base)
{
	int tz;
	char p;
	tm t;
	t.tm_isdst = 0;
	sscanf(str, "%2d%2d%2d%2d%2d%2d0%2d%c", &t.tm_year, &t.tm_mon, &t.tm_mday,
		&t.tm_hour, &t.tm_min, &t.tm_sec, &tz, &p);
	switch (p)
	{
		case '+':
			t.tm_year += 100;
			t.tm_mon -= 1;
			return (mktime(&t) + timezone + tz * 900);
		case '-':
			t.tm_year += 100;
			t.tm_mon -= 1;
			return (mktime(&t) + timezone - tz * 900);
		case 'R':
			return (base + t.tm_year * 31104000 + t.tm_mon * 2592000 +
				t.tm_mday * 86400 + t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec);
		default:
			__unreachable__("Invalid time format");
	}
}

bool SmppUtil::compareAddresses(PduAddress& a1, PduAddress& a2)
{
	return (a1.get_typeOfNumber() == a2.get_typeOfNumber() &&
		a1.get_numberingPlan() == a2.get_numberingPlan() &&
		strcmp(a1.get_value(), a2.get_value()) == 0);
}

#define __compare_optional__(field, expr, errCode) \
	if ((opt1.has_##field() && !opt2.has_##field()) || \
		(!opt1.has_##field() && opt2.has_##field()) || \
		(opt1.has_##field() && opt2.has_##field() && (expr))) \
	{ res.push_back(errCode); }

#define __compare_optional_int__(field, errCode) \
	__compare_optional__(field, opt1.get_##field() != opt2.get_##field(), errCode)

#define __compare_optional_intarr__(field, errCode) \
	__compare_optional__(field, sizeof(opt1.get_##field()) != sizeof(opt2.get_##field()) || \
		memcmp(opt1.get_##field(), opt2.get_##field(), sizeof(opt1.get_##field())), errCode)

#define __compare_optional_cstr__(field, errCode) \
	__compare_optional__(field, !strcmp(opt1.get_##field(), opt2.get_##field()), errCode)

#define __compare_optional_ostr__(field, errCode) \
	__compare_optional__(field, opt1.size_##field() != opt2.size_##field() || \
		strncmp(opt1.get_##field(), opt2.get_##field(), opt1.size_##field()), errCode)

vector<int> SmppUtil::compareOptional(SmppOptional& opt1, SmppOptional& opt2)
{
	vector<int> res;
	__compare_optional_int__(destAddrSubunit, 1);
	__compare_optional_int__(sourceAddrSubunit, 2);
	__compare_optional_int__(destNetworkType, 3);
	__compare_optional_int__(sourceNetworkType, 4);
	__compare_optional_int__(destBearerType, 5);
	__compare_optional_int__(sourceBearerType, 6);
	__compare_optional_int__(destTelematicsId, 7);
	__compare_optional_int__(sourceTelematicsId, 8);
	__compare_optional_int__(qosTimeToLive, 9);
	__compare_optional_int__(payloadType, 10);
	__compare_optional_cstr__(additionalStatusInfoText, 11);
	__compare_optional_cstr__(receiptedMessageId, 12);
	__compare_optional_int__(msMsgWaitFacilities, 13);
	__compare_optional_int__(privacyIndicator, 14);
	__compare_optional_ostr__(sourceSubaddress, 15);
	__compare_optional_ostr__(destSubaddress, 16);
	__compare_optional_int__(userMessageReference, 17);
	__compare_optional_int__(userResponseCode, 18);
	__compare_optional_int__(languageIndicator, 19);
	__compare_optional_int__(sourcePort, 20);
	__compare_optional_int__(destinationPort, 21);
	__compare_optional_int__(sarMsgRefNum, 22);
	__compare_optional_int__(sarTotalSegments, 23);
	__compare_optional_int__(sarSegmentSegnum, 24);
	__compare_optional_int__(scInterfaceVersion, 25);
	__compare_optional_int__(displayTime, 26);
	__compare_optional_int__(msValidity, 27);
	__compare_optional_int__(dpfResult, 28);
	__compare_optional_int__(setDpf, 29);
	__compare_optional_int__(msAvailableStatus, 30);
	__compare_optional_intarr__(networkErrorCode, 31);
	__compare_optional_ostr__(messagePayload, 32);
	__compare_optional_int__(deliveryFailureReason, 33);
	__compare_optional_int__(moreMessagesToSend, 34);
	__compare_optional_int__(messageState, 35);
	__compare_optional_ostr__(callbackNum, 36);
	__compare_optional_int__(callbackNumPresInd, 37);
	__compare_optional_ostr__(callbackNumAtag, 38);
	__compare_optional_int__(numberOfMessages, 39);
	__compare_optional_int__(smsSignal, 40);
	__compare_optional_int__(alertOnMessageDelivery, 41);
	__compare_optional_int__(itsReplyType, 42);
	__compare_optional_intarr__(itsSessionInfo, 43);
	__compare_optional_int__(ussdServiceOp, 44);
	return res;
}

int SmppUtil::setupRandomCorrectShortMessage(ShortMessage* msg)
{
	if (msg)
	{
		int len = rand1(MAX_SHORT_MESSAGE_LENGTH);
		rand_char(len, *msg);
		return len;
	}
}

#define __set_message__(field, value) \
	pdu->get_message().set_##field(value)

void SmppUtil::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu)
{
	SmppTime tmp;
	//PduPartSm
	EService serviceType;
	rand_char(MAX_ESERVICE_TYPE_LENGTH, serviceType);
	__set_message__(serviceType, serviceType);
	//__set_message__(source, ...);
	//__set_message__(dest, ...);
	__set_message__(esmClass, rand0(255));
	__set_message__(protocolId, rand0(255));
	__set_message__(priorityFlag, rand0(255));
	time_t waitTime = time(NULL) + rand1(60);
	time_t validTime = waitTime + rand1(60);
	__set_message__(scheduleDeliveryTime, time2string(waitTime, tmp, time(NULL), __numTime__));
	__set_message__(validityPeriod, time2string(validTime, tmp, time(NULL), __numTime__));
	__set_message__(registredDelivery, rand0(255));
	__set_message__(replaceIfPresentFlag, !rand0(10));
	__set_message__(dataCoding, rand0(255));
	__set_message__(smDefaultMsgId, rand0(255)); //хбз что это такое
	ShortMessage msg;
	int len = setupRandomCorrectShortMessage(&msg);
	pdu->get_message().set_shortMessage(msg, len);
	//SmppOptional
	setupRandomCorrectOptionalParams(pdu->get_optional());
}

#undef __set_message__
#define __set_message__(field, value) \
	pdu->set_##field(value)

void SmppUtil::setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu)
{
	SmppTime tmp;
	//__set_message__(messageId, ...);
	//__set_message__(source, ...);
	time_t waitTime = time(NULL) + rand1(60);
	time_t validTime = waitTime + rand1(60);
	__set_message__(scheduleDeliveryTime, time2string(waitTime, tmp, time(NULL), __numTime__));
	__set_message__(validityPeriod, time2string(validTime, tmp, time(NULL), __numTime__));
	__set_message__(registredDelivery, rand0(255));
	__set_message__(smDefaultMsgId, rand0(255)); //хбз что это такое
	ShortMessage msg;
	int len = setupRandomCorrectShortMessage(&msg);
	pdu->set_shortMessage(msg, len);
}

#define __set_optional__(field, value) \
	if ((mask >>= 1) & 0x1) { \
		__trace__("set_optional: " #field); \
		opt.set_##field(value); \
	}

#define __set_optional_ostr__(field, len) \
	if ((mask >>= 1) & 0x1) { \
		__trace__("set_optional_ostr: " #field); \
		auto_ptr<char> str = rand_char(len); \
		opt.set_##field(str.get(), len); \
	}

#define __set_optional_cstr__(field, len) \
	if ((mask >>= 1) & 0x1) { \
		__trace__("set_optional_cstr: " #field); \
		auto_ptr<char> str = rand_char(len); \
		opt.set_##field(str.get()); \
	}

void SmppUtil::setupRandomCorrectOptionalParams(SmppOptional& opt, uint64_t mask)
{
	auto_ptr<uint8_t> tmp = rand_uint8_t(8);
	mask &= *((uint64_t*) tmp.get());
	__trace2__("SmppUtil::setupRandomCorrectOptionalParams(): mask = %#llu", mask);
	__set_optional__(destAddrSubunit, rand0(255));
	__set_optional__(sourceAddrSubunit, rand0(255));
	__set_optional__(destNetworkType, rand0(255));
	__set_optional__(sourceNetworkType, rand0(255));
	__set_optional__(destBearerType, rand0(255));
	__set_optional__(sourceBearerType, rand0(255));
	__set_optional__(destTelematicsId, rand0(65535));
	__set_optional__(sourceTelematicsId, rand0(255));
	__set_optional__(qosTimeToLive, rand0(INT_MAX));
	__set_optional__(payloadType, rand0(255));
	__set_optional_cstr__(additionalStatusInfoText, rand1(255));
	MessageId msgId;
	__set_optional__(receiptedMessageId, convert(rand1(INT_MAX), msgId));
	__set_optional__(msMsgWaitFacilities, rand0(255));
	__set_optional__(privacyIndicator, rand0(255));
	__set_optional_ostr__(sourceSubaddress, rand2(2, 22));
	__set_optional_ostr__(destSubaddress, rand2(2, 22));
	__set_optional__(userMessageReference, rand0(65535));
	__set_optional__(userResponseCode, rand0(255));
	__set_optional__(languageIndicator, rand0(255));
	__set_optional__(sourcePort, rand0(65535));
	__set_optional__(destinationPort, rand0(65535));
	__set_optional__(sarMsgRefNum, rand0(65535));
	__set_optional__(sarTotalSegments, rand1(255));
	__set_optional__(sarSegmentSegnum, rand1(255));
	__set_optional__(scInterfaceVersion, rand0(255));
	__set_optional__(displayTime, rand0(255));
	__set_optional__(msValidity, rand0(255));
	__set_optional__(dpfResult, rand0(255));
	__set_optional__(setDpf, rand0(255));
	__set_optional__(msAvailableStatus, rand0(255));
	int errCode = rand0(INT_MAX);
	__set_optional__(networkErrorCode, (uint8_t*) &errCode);
	__set_optional_ostr__(messagePayload, rand0(65535));
	__set_optional__(deliveryFailureReason, rand0(255));
	__set_optional__(moreMessagesToSend, rand0(255));
	__set_optional__(messageState, rand0(255));
	__set_optional_ostr__(callbackNum, rand2(4, 19));
	__set_optional__(callbackNumPresInd, rand0(255));
	__set_optional_ostr__(callbackNumAtag, rand0(65));
	__set_optional__(numberOfMessages, rand0(255));
	__set_optional__(smsSignal, rand0(65535));
	__set_optional__(alertOnMessageDelivery, rand0(1));
	__set_optional__(itsReplyType, rand0(255));
	int sessInfo = rand0(INT_MAX);
	__set_optional__(itsSessionInfo, (uint8_t*) &sessInfo);
	__set_optional__(ussdServiceOp, rand0(255));
}

/*
void SmppUtil::setupRandomCorrectSubmitMultiPdu(PduMultiSm* pdu)
{
	//PduPartSm
	__set_message__(serviceType(c_str);
	__set_message__(source(PduAddress);
	__set_message__(numberOfDests(uint8_t);
	__set_message__(dests(PduDestAddress);
	__set_message__(esmClass(uint8_t);
	__set_message__(protocolId(uint8_t);
	__set_message__(priorityFlag(uint8_t);
	__set_message__(scheduleDeliveryTime(c_str);
	__set_message__(validityPeriod(c_str);
	__set_message__(registredDelivery(uint8_t);
	__set_message__(replaceIfPresentFlag(uint8_t);
	__set_message__(dataCoding(uint8_t);
	__set_message__(smDefaultMsgId(uint8_t);
	__set_message__(shortMessage(const char* __value,int __len) ;
	//SmppOptional
}
*/

}
}
}

