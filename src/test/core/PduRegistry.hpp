#ifndef TEST_CORE_PDU_REGISTRY
#define TEST_CORE_PDU_REGISTRY

#include "PduUtil.hpp"
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

/**
 * Реестр отправленных pdu.
 * Все методы несинхронизованы и требуют внешней синхронизации.
 */
class PduRegistry
{
	typedef map<const string, PduData*> SmsIdMap;
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
	PduData* lastRemovedPduData;

	SmsIdMap idMap;
	SeqNumMap seqNumMap;
	MsgRefMap msgRefMap;
	TimeMap submitTimeMap;
	TimeMap waitTimeMap;
	TimeMap validTimeMap;

	const char* toString(PduData* pduData) const;

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

	PduRegistry() : msgRef(0), lastRemovedPduData(NULL) {}
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
	 * Поиск оригинального pdu при получении его на стороне получателя.
	 */
	PduData* getPdu(uint16_t msgRef);

	/**
	 * Поиск оригинального pdu при получении респонса на стороне отправителя.
	 */
	PduData* getPdu(uint32_t seqNumber);
	
	/**
	 * Поиск оригинального pdu при получении статус репорта или
	 * нотификации на стороне отправителя.
	 * Используется для:
	 * <ul>
	 * <li>SMSC Delivery Receipt.
	 * <li>Intermediate Notification.
	 * <li>SME Delivery Acknowledgement.
	 * <li>SME Manual/User Acknowledgement.
	 * </ul>
	 */
	PduData* getPdu(const string& smsId);

	void removePdu(PduData* pduData);

	PduData* getLastRemovedPdu();

	PduDataIterator* getPduBySubmitTime(time_t t1, time_t t2);
	
	PduDataIterator* getPduByWaitTime(time_t t1, time_t t2);

	PduDataIterator* getPduByValidTime(time_t t1, time_t t2);

	int size() const;

	void dump(FILE* log) const;
};

}
}
}

#endif /* TEST_CORE_PDU_REGISTRY */
