#ifndef TEST_SMPP_SMPP_UTIL
#define TEST_SMPP_SMPP_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace smpp {

using std::string;
using smsc::sms::Address;
using smsc::sms::SMSId;
using namespace smsc::smpp;
using namespace smsc::test::sms; //constants

#define __numTime__ rand1(3)
#define __absoluteTime__ rand1(2)

#define __dumpSubmitSmPdu__(tc, id, pdu) \
	if (pdu) { \
		time_t lt = time(NULL); tm t; char buf[30]; \
		__trace2__("%s(): systemId = %s, sequenceNumber = %u, scheduleDeliveryTime = %ld, validityPeriod = %ld, system time = %s", \
			tc, id.c_str(), (pdu)->get_header().get_sequenceNumber(), \
			SmppUtil::getWaitTime((pdu)->get_message().get_scheduleDeliveryTime(), time(NULL)), \
			SmppUtil::getValidTime((pdu)->get_message().get_validityPeriod(), time(NULL)), \
			asctime_r(localtime_r(&lt, &t), buf)); \
		(pdu)->dump(TRACE_LOG_STREAM); \
	} else { \
		__trace2__("%s(): pdu = NULL", tc); \
	}

#define __dumpReplaceSmPdu__(tc, id, pdu) \
	if (pdu) { \
		time_t lt = time(NULL); tm t; char buf[30]; \
		__trace2__("%s(): systemId = %s, sequenceNumber = %u, scheduleDeliveryTime = %ld, validityPeriod = %ld, system time = %s", \
			tc, id.c_str(), (pdu)->get_header().get_sequenceNumber(), \
			SmppUtil::getWaitTime((pdu)->get_scheduleDeliveryTime(), time(NULL)), \
			SmppUtil::getValidTime((pdu)->get_validityPeriod(), time(NULL)), \
			asctime_r(localtime_r(&lt, &t), buf)); \
		(pdu)->dump(TRACE_LOG_STREAM); \
	} else { \
		__trace2__("%s(): pdu = NULL", tc); \
	}

#define __dumpPdu__(tc, id, pdu) \
	if (pdu) { \
		time_t lt = time(NULL); tm t; char buf[30]; \
		__trace2__("%s(): systemId = %s, sequenceNumber = %u, system time = %s", \
			tc, id.c_str(), (pdu)->get_header().get_sequenceNumber(), \
			asctime_r(localtime_r(&lt, &t), buf)); \
		(pdu)->dump(TRACE_LOG_STREAM); \
	} else { \
		__trace2__("%s(): pdu = NULL", tc); \
	}

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
//��������� ����: SME Delivery Acknowledgement, SME Manual/User Acknowledgment,
//Conversation Abort (Korean CDMA) ���������
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
//SME originated Acknowledgement - ����������
//Intermediate Notification:
//xxx0xxxx - No Intermediate notification requested
//xxx1xxxx - Intermediate notification requested
const uint8_t INTERMEDIATE_NOTIFICATION_REQUESTED = 0x10;

//������� �������� � ���� ���������
const int MAX_SMPP_TIME_LENGTH = 20;
typedef char SmppTime[MAX_SMPP_TIME_LENGTH];
typedef char ShortMessage[MAX_SM_LENGTH];
typedef char MessageId[MAX_MSG_ID_LENGTH + 1];

class SmppUtil
{
	static time_t string2time(const char* str, time_t base, bool check = true);

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
	/*
	static SMSId convert(const char* id);
	static MessageId& convert(const SMSId& smsId, MessageId& smppId);
	*/
	static const char* time2string(time_t t, char* str, time_t base, int num, bool check = true);
	static time_t getWaitTime(const char* str, time_t submitTime);
	static time_t getValidTime(const char* str, time_t submitTime);

	//static bool compareAddresses(PduAddress& a1, PduAddress& a2);
	
	//��������� ������ vector<int> operator==(SmppOptional&, SmppOptional&)
	//��� ����������� ���������� ��������� compareFlag
	static vector<int> compareOptional(SmppOptional& opt1, SmppOptional& opt2);

	static void setupRandomCorrectAddress(PduAddress* addr, bool check = true);
	static void setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu,
		uint64_t mask = 0xffffffffffffffff, bool check = true);
	static void setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu,
		uint64_t mask = 0xffffffffffffffff, bool check = true);
	static void setupRandomCorrectOptionalParams(SmppOptional& opt,
		uint64_t mask = 0xffffffffffffffff, bool check = true);

};

bool operator==(PduAddress& a1, PduAddress& a2);
bool operator!=(PduAddress& a1, PduAddress& a2);

}
}
}

#endif /* TEST_SMPP_SMPP_UTIL */
