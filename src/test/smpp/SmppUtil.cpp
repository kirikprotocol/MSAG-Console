#include "SmppUtil.hpp"
#include "test/util/Util.hpp"
#include "sms/sms.h"
#include <ctime>

#define __numTime__ rand1(2)

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

MessageId* SmppUtil::convert(const SMSId& smsId, MessageId* smppId)
{
	sprintf(*smppId, "%d", smsId);
	return smppId;
}

const char* SmppUtil::time2string(time_t lt, char* str, int num)
{
	switch (num)
	{
		case 1: //Absolute time format
			{
				int len = strftime(str, MAX_SMPP_TIME_LENGTH,
					"%y%m%d%H%M%S0", gmtime(&lt));
				sprintf(str + len, "%02d+", timezone / 900);
			}
			break;
		case 2: //Relative time format
			{
				time_t df = lt - time(NULL);
				__require__(df > 0);
				tm t;
				t.tm_year = (df / 31104000);
				t.tm_mon = (df / 2592000) % 12;
				t.tm_mday = (df / 86400) % 30;
				t.tm_hour = (df / 3600) % 24;
				t.tm_min = (df / 60) % 60;
				t.tm_sec = df % 60;
				//t.tm_wday; t.tm_yday; t.tm_isdst;
				strftime(str, MAX_SMPP_TIME_LENGTH, "%y%m%d%H%M%S000R", &t);
			}
			break;
		default:
			throw "";
	}
	return str;
}

time_t SmppUtil::string2time(const char* str)
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
		case '-':
			t.tm_year += 100;
			t.tm_mon -= 1;
			return (mktime(&t) - timezone);
		case 'R':
			return (time(NULL) + t.tm_year * 31104000 + t.tm_mon * 2592000 +
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

vector<int> SmppUtil::compareOptional(SmppOptional& opt1, SmppOptional& opt2)
{
	vector<int> res;
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

void SmppUtil::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu)
{
	SmppTime tmp;
	//PduPartSm
	EService serviceType;
	rand_char(MAX_ESERVICE_TYPE_LENGTH, serviceType);
	pdu->get_message().set_serviceType(serviceType);
	//pdu->get_message().set_source(PduAddress);
	//pdu->get_message().set_dest(PduAddress);
	pdu->get_message().set_esmClass(rand0(255));
	pdu->get_message().set_protocolId(rand0(255));
	pdu->get_message().set_priorityFlag(rand0(255));
	time_t waitTime = time(NULL) + rand1(60);
	time_t validTime = waitTime + rand1(60);
	pdu->get_message().set_scheduleDeliveryTime(
		time2string(waitTime, tmp, __numTime__));
	pdu->get_message().set_validityPeriod(
		time2string(validTime, tmp, __numTime__));
	pdu->get_message().set_registredDelivery(rand0(255));
	pdu->get_message().set_replaceIfPresentFlag(!rand0(10));
	ShortMessage msg;
	int len = setupRandomCorrectShortMessage(&msg);
	pdu->get_message().set_dataCoding(rand0(255));
	pdu->get_message().set_smDefaultMsgId(0); //хбз что это такое
	pdu->get_message().set_shortMessage(msg, len);
	//SmppOptional
}

/*
void SmppUtil::setupRandomCorrectSubmitMultiPdu(PduMultiSm* pdu)
{
	//PduPartSm
	pdu->get_message().set_serviceType(c_str);
	pdu->get_message().set_source(PduAddress);
	pdu->get_message().set_numberOfDests(uint8_t);
	pdu->get_message().set_dests(PduDestAddress);
	pdu->get_message().set_esmClass(uint8_t);
	pdu->get_message().set_protocolId(uint8_t);
	pdu->get_message().set_priorityFlag(uint8_t);
	pdu->get_message().set_scheduleDeliveryTime(c_str);
	pdu->get_message().set_validityPeriod(c_str);
	pdu->get_message().set_registredDelivery(uint8_t);
	pdu->get_message().set_replaceIfPresentFlag(uint8_t);
	pdu->get_message().set_dataCoding(uint8_t);
	pdu->get_message().set_smDefaultMsgId(uint8_t);
	pdu->get_message().set_shortMessage(const char* __value,int __len) ;
	//SmppOptional
}
*/

}
}
}

