#ifndef TEST_SMPP_SMPP_UTIL
#define TEST_SMPP_SMPP_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace smpp {

using smsc::sms::Address;
using smsc::sms::SMSId;
using namespace smsc::smpp;
using namespace smsc::test::sms; //constants

#define __maxValidPeriod__ (time(NULL) + 7 * 24 * 3600) //7 суток
#define __numTime__ rand1(2)
#define __absoluteTime__ 1

typedef enum
{ 
	SMPP_ENROUTE_STATE = 1,
	SMPP_DELIVERED_STATE = 2,
	SMPP_EXPIRED_STATE = 3,
	SMPP_DELETED_STATE = 4,
	SMPP_UNDELIVERABLE_STATE = 5,
	SMPP_ACCEPTED_STATE = 6,
	SMPP_UNKNOWN_STATE = 7,
	SMPP_REJECTED_STATE = 8
} SmppState;

//esm_class parameter for SMSC -> ESME PDUs (SMPP v3.4, 5.2.12):
//ESME -> SMSC PDUs:
//xxxxxx00 - Default SMSC Mode (e.g. Store and Forward)
//xxxxxx01 - Datagram mode
//xxxxxx10 - Forward (i.e. Transaction) mode
//xxxxxx11 - Store and Forward mode
const uint8_t ESM_CLASS_DEFAULT_SMSC_MODE = 0x0;
const uint8_t ESM_CLASS_DATAGRAM_MODE = 0x1;
const uint8_t ESM_CLASS_FORWARD_MODE = 0x2;
const uint8_t ESM_CLASS_STORE_AND_FORWARD_MODE = 0x3;
//SMSC -> ESME PDUs:
//xx0000xx - Default message Type (i.e. normal message)
//xx0001xx - Short Message contains SMSC Delivery Receipt
//xx1000xx - Short Message contains Intermediate Delivery Notification
//остальные типы: SME Delivery Acknowledgement, SME Manual/User Acknowledgment,
//Conversation Abort (Korean CDMA) игнорирую
const uint8_t ESM_CLASS_MESSAGE_TYPE_BITS = 0x3c;
const uint8_t ESM_CLASS_NORMAL_MESSAGE = 0x0;
const uint8_t ESM_CLASS_DELIVERY_RECEIPT = 0x4;
const uint8_t ESM_CLASS_INTERMEDIATE_NOTIFICATION = 0x20;

//registered_delivery (SMPP v3.4, 5.2.17):
//SMSC Delivery Receipt:
//xxxxxx00 - No SMSC Delivery Receipt requested (default)
//xxxxxx01 - SMSC Delivery Receipt requested where final delivery outcome
//           is delivery success or failure
//xxxxxx10 - SMSC Delivery Receipt requested where the final delivery
//           outcome is delivery failure
const uint8_t SMSC_DELIVERY_RECEIPT_BITS = 0x3;
const uint8_t NO_SMSC_DELIVERY_RECEIPT = 0x0;
const uint8_t FINAL_SMSC_DELIVERY_RECEIPT = 0x1;
const uint8_t FAILURE_SMSC_DELIVERY_RECEIPT = 0x2;
//SME originated Acknowledgement - игнорируем
//Intermediate Notification:
//xxx0xxxx - No Intermediate notification requested
//xxx1xxxx - Intermediate notification requested
const uint8_t INTERMEDIATE_NOTIFICATION_REQUESTED = 0x10;

//размеры структур и сами структуры
const int MAX_SMPP_TIME_LENGTH = 20;
const int MAX_SMS_ID_LENGTH = 65;
typedef char SmppTime[MAX_SMPP_TIME_LENGTH];
typedef char ShortMessage[MAX_MSG_BODY_LENGTH];
typedef char MessageId[MAX_SMS_ID_LENGTH];

class SmppUtil
{
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
	static Address* convert(PduAddress& smppAddr, Address* smsAddr);
	static SMSId convert(const char* id);
	static MessageId* convert(const SMSId& smsId, MessageId* smppId);
	static const char* time2string(time_t t, char* str, int num);
	static time_t string2time(const char* str);

	static bool compareAddresses(PduAddress& a1, PduAddress& a2);
	static vector<int> compareOptional(SmppOptional& opt1, SmppOptional& opt2);

	static int setupRandomCorrectShortMessage(ShortMessage* msg);
	static void setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu);
};

}
}
}

#endif /* TEST_SMPP_SMPP_UTIL */
