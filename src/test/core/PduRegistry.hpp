#ifndef TEST_CORE_PDU_REGISTRY
#define TEST_CORE_PDU_REGISTRY

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include "core/synchronization/Mutex.hpp"
#include <map>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::map;
using std::multimap;
using std::pair;
using std::vector;
using smsc::sms::SMSId;
using smsc::sms::Address;
using smsc::smpp::SmppHeader;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

static const int PDU_REQUIRED_FLAG = 0x0; //pdu ожидаетс€, но еще не получена
static const int PDU_RECEIVED_FLAG = 0x1; //pdu получена воврем€
//pdu получена вне ожидаемого интервала или вообще не получена
static const int PDU_MISSING_ON_TIME_FLAG = 0x2;
static const int PDU_NOT_EXPECTED_FLAG = 0x3; //данной pdu быть не должно

/**
 * —труктура дл€ хранени€ данных pdu.
 */
struct PduData
{
	SMSId smsId;
	const uint16_t msgRef;
	const time_t submitTime;
	const time_t waitTime;
	const time_t validTime;
	SmppHeader* pdu;
	int responseFlag; //флаг получени€ респонса
	int deliveryFlag; //флаг получени€ сообщени€ плучателем
	int deliveryReceiptFlag; //флаг получени€ подтверждени€ доставки
	int intermediateNotificationFlag; //флаг получени€ всех нотификаций
	PduData* replacePdu; //pdu, котора€ должна быть заменена текущей pdu
	PduData* replacedByPdu; //pdu, котора€ замещает текущую pdu

	PduData(uint16_t _msgRef, time_t _submitTime, time_t _waitTime,
		time_t _validTime, SmppHeader* _pdu)
		: smsId(0), msgRef(_msgRef), submitTime(_submitTime),
		waitTime(_waitTime), validTime(_validTime),
		pdu(_pdu), responseFlag(PDU_REQUIRED_FLAG),
		deliveryFlag(PDU_REQUIRED_FLAG), deliveryReceiptFlag(PDU_REQUIRED_FLAG),
		intermediateNotificationFlag(PDU_REQUIRED_FLAG), replacePdu(NULL),
		replacedByPdu(NULL) {}

	PduData(const PduData& data)
		: smsId(data.smsId), msgRef(data.msgRef), submitTime(data.submitTime),
		waitTime(data.waitTime), validTime(data.validTime),
		pdu(data.pdu), responseFlag(data.responseFlag),
		deliveryFlag(data.deliveryFlag),
		deliveryReceiptFlag(data.deliveryReceiptFlag),
		intermediateNotificationFlag(data.intermediateNotificationFlag),
		replacePdu(data.replacePdu), replacedByPdu(data.replacedByPdu) {}

	~PduData()
	{
		//disposePdu(pdu);
	}

	bool complete()
	{
		return responseFlag && deliveryFlag && deliveryReceiptFlag &&
			intermediateNotificationFlag;
	}
};

/**
 * –еестр отправленных pdu.
 * ¬се методы несинхронизованы и требуют внешней синхронизации.
 */
class PduRegistry
{
	typedef map<const SMSId, PduData*> SmsIdMap;
	typedef map<const uint32_t, PduData*> SeqNumMap;
	typedef map<const uint16_t, PduData*> MsgRefMap;

	struct TimeKey
	{
		time_t time;
		uint16_t msgRef;
		TimeKey(time_t t, uint16_t _msgRef) : time(t), msgRef(_msgRef) {}
		bool operator< (const TimeKey& key) const
		{
			return (time == key.time ? (msgRef < key.msgRef) : (time < key.time));
		}
	};
	typedef map<const TimeKey, PduData*> TimeMap;

	Mutex mutex;
	uint16_t msgRef;

	SmsIdMap idMap;
	SeqNumMap seqNumMap;
	MsgRefMap msgRefMap;
	TimeMap submitTimeMap;
	TimeMap waitTimeMap;
	TimeMap validTimeMap;

public:
	struct PduDataIterator
	{
		TimeMap::iterator it1;
		TimeMap::iterator it2;
		PduDataIterator(TimeMap::iterator i1, TimeMap::iterator i2)
			: it1(i1), it2(i2) {}
		virtual PduData* next()
		{
			return (it1 != it2 ? (it1++)->second : NULL);
		}
	};

	PduRegistry() : msgRef(0) {}
	~PduRegistry();

	Mutex& getMutex()
	{
		return mutex;
	}

	uint16_t nextMsgRef()
	{
		MutexGuard guard(mutex);
		return ++msgRef;
	}

	void registerPdu(PduData* pduData);

	void updatePdu(PduData* pduData);

	void clear();

	/**
	 * ѕоиск оригинального pdu при получении его на стороне получател€.
	 */
	PduData* getPdu(uint16_t msgRef);

	/**
	 * ѕоиск оригинального pdu при получении респонса на стороне отправител€.
	 */
	PduData* getPdu(uint32_t seqNumber);
	
	/**
	 * ѕоиск оригинального pdu при получении статус репорта или
	 * нотификации на стороне отправител€.
	 * »спользуетс€ дл€:
	 * <ul>
	 * <li>SMSC Delivery Receipt.
	 * <li>Intermediate Notification.
	 * <li>SME Delivery Acknowledgement.
	 * <li>SME Manual/User Acknowledgement.
	 * </ul>
	 */
	PduData* getPdu(const SMSId smsId);

	void removePdu(PduData* pduData);

	PduDataIterator* getPduBySubmitTime(time_t t1, time_t t2);
	
	PduDataIterator* getPduByWaitTime(time_t t1, time_t t2);

	PduDataIterator* getPduByValidTime(time_t t1, time_t t2);

	int size();

	void dump(FILE* log);
};

}
}
}

#endif /* TEST_CORE_PDU_REGISTRY */
