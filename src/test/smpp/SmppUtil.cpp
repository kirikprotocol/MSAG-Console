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

Address* SmppUtil::convert(const PduAddress& smppAddr, Address* smsAddr)
{
	smsAddr->setTypeOfNumber(smppAddr->get_typeOfNumber());
	smsAddr->setNumberingPlan(smppAddr->get_numberingPlan());
	const char* val = smppAddr->get_value();
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
				sprintf(str + len, "%02d+", timezone * 4);
			}
			break;
		case 2: //Relative time format
			{
				time_t df = lt - time(NULL);
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

vector<int> comparePdu(const SmppHeader& pdu1, const SmppHeader& pdu2)
{
	vector<int> res;
	int i = 0;

#define __compare__(prop) \
	if (pdu1.get_prop() != pdu2.get_prop()) res.push_back(++i);
	
	//header
	__compare__(commandLength);
	__compare__(commandId);
	__compare__(commandStatus);
	__compare__(sequenceNumber);
	if (i)
	{
		return res;
	}
	//
	switch ( _pdu->commandId )
	{
  case GENERIC_NACK:  return reinterpret_cast<PduGenericNack*>(_pdu)->size();
  case BIND_RECIEVER: return reinterpret_cast<PduBindTRX*>(_pdu)->size();
  case BIND_RECIEVER_RESP: return reinterpret_cast<PduBindTRXResp*>(_pdu)->size();
  case BIND_TRANSMITTER: return reinterpret_cast<PduBindTRX*>(_pdu)->size();
  case BIND_TRANSMITTER_RESP: return reinterpret_cast<PduBindTRXResp*>(_pdu)->size();
  case QUERY_SM: return reinterpret_cast<PduQuerySm*>(_pdu)->size();
  case QUERY_SM_RESP: return reinterpret_cast<PduQuerySmResp*>(_pdu)->size();
  case SUBMIT_SM: return reinterpret_cast<PduXSm*>(_pdu)->size(false);
  case SUBMIT_SM_RESP: return reinterpret_cast<PduSubmitSmResp*>(_pdu)->size();
  case DELIVERY_SM: return reinterpret_cast<PduXSm*>(_pdu)->size(false);
  case DELIVERY_SM_RESP: return reinterpret_cast<PduDeliverySmResp*>(_pdu)->size();
  case UNBIND: return reinterpret_cast<PduUnbind*>(_pdu)->size();
  case UNBIND_RESP: return reinterpret_cast<PduUnbindResp*>(_pdu)->size();
  case REPLACE_SM: return reinterpret_cast<PduReplaceSm*>(_pdu)->size();
  case REPLACE_SM_RESP: return reinterpret_cast<PduReplaceSmResp*>(_pdu)->size();
  case CANCEL_SM: return reinterpret_cast<PduCancelSm*>(_pdu)->size();
  case CANCEL_SM_RESP: return reinterpret_cast<PduCancelSmResp*>(_pdu)->size();
  case BIND_TRANCIEVER: return reinterpret_cast<PduBindTRX*>(_pdu)->size();
  case BIND_TRANCIEVER_RESP: return reinterpret_cast<PduBindTRXResp*>(_pdu)->size();
  case OUTBIND: return reinterpret_cast<PduOutBind*>(_pdu)->size();
  case ENQUIRE_LINK: return reinterpret_cast<PduEnquireLink*>(_pdu)->size();
  case ENQUIRE_LINK_RESP: return reinterpret_cast<PduEnquireLinkResp*>(_pdu)->size();
  case SUBMIT_MULTI: return reinterpret_cast<PduXSm*>(_pdu)->size(true);
  case SUBMIT_MULTI_RESP: return reinterpret_cast<PduMultiSmResp*>(_pdu)->size();
  case ALERT_NOTIFICATION: return reinterpret_cast<PduAlertNotification*>(_pdu)->size();
  case DATA_SM: return reinterpret_cast<PduDataSm*>(_pdu)->size();
  case DATA_SM_RESP: return reinterpret_cast<PduDataSmResp*>(_pdu)->size();




	if (pdu1.get_() != pdu2.get_())
	{
		res.push_back();
	}
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
	//pdu->get_message().set_serviceType(const char*);
	//pdu->get_message().set_source(PduAddress);
	//pdu->get_message().set_dest(PduAddress);
	pdu->get_message().set_esmClass(rand0(255));
	pdu->get_message().set_protocolId(rand0(255));
	pdu->get_message().set_priorityFlag(rand0(255));
	pdu->get_message().set_scheduleDeliveryTime(time2string(time(NULL), tmp, rand1(2)));
	pdu->get_message().set_validityPeriod(time2string(time(NULL) + 10, tmp, rand1(2)));
	pdu->get_message().set_registredDelivery(rand0(255));
	pdu->get_message().set_replaceIfPresentFlag(0); //Don’t replace
	ShortMessage msg;
	int len = setupRandomCorrectShortMessage(&msg);
	pdu->get_message().set_dataCoding(rand0(255));
	pdu->get_message().set_smDefaultMsgId(0); //õáç ÷òî ýòî òàêîå
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

