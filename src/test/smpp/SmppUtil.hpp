#ifndef TEST_SMPP_SMPP_UTIL
#define TEST_SMPP_SMPP_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace smpp {

using namespace smsc::smpp;
using namespace smsc::test::sms; //constants

const int MAX_SMPP_TIME_LENGTH = 20;
const int MAX_SMS_ID_LENGTH = 65;
typedef char SmppTime[MAX_SMPP_TIME_LENGTH];
typedef char ShortMessage[MAX_MSG_BODY_LENGTH];
typedef char MessageId[MAX_SMS_ID_LENGTH];

class SmppUtil
{
	static const int timezone = 7;

public:
	/*
	virtual SmppHeader* sendPdu(SmppHeader& pdu)=0;
	virtual void sendGenericNack(PduGenericNack& pdu)=0;
	virtual void sendDeliverySmResp(PduDeliverySmResp& pdu)=0;
	virtual void sendDataSmResp(PduDataSmResp& pdu)=0;
	virtual PduSubmitSmResp* submit()=0;
	virtual PduMultiSmResp* submitm(PduMultiSm& pdu)=0;
	virtual PduDataSmResp* data(PduDataSm& pdu)=0;
	virtual PduQuerySmResp* query(PduQuerySm& pdu)=0;
	virtual PduCancelSmResp* cancel(PduCancelSm& pdu)=0;
	virtual PduReplaceSmResp* replace(PduReplaceSm& pdu)=0;
	*/
	
	static PduAddress* convert(const Address& smsAddr, PduAddress* smppAddr);
	static Address* convert(const PduAddress& smppAddr, Address* smsAddr);
	static SMSId convert(const char* id);
	static MessageId* convert(const SMSId& smsId, MessageId* smppId);
	static const char* time2string(time_t lt, char* str, int num);

	static vector<int> comparePdu(const SmppHeader& pdu1, const SmppHeader& pdu2);

	static int setupRandomCorrectShortMessage(ShortMessage* msg);
	static void setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu);
};

}
}
}

#endif /* TEST_SMPP_SMPP_UTIL */
