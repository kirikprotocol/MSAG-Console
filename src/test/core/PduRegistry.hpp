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
	//typedef map<const string, PduData*> SmsIdMap;
	typedef map<const uint32_t, ResponseMonitor*> SeqNumMap;

	struct MsgRefKey
	{
		const uint16_t msgRef;
		const uint32_t id;
		MsgRefKey(uint16_t _msgRef, uint32_t _id) : msgRef(_msgRef), id(_id) {}
		bool operator< (const MsgRefKey& key) const
		{
			return (msgRef == key.msgRef ? (id < key.id) : (msgRef < key.msgRef));
		}
	};
	typedef map<const MsgRefKey, PduMonitor*> MsgRefMap;

	struct TimeKey
	{
		const time_t time;
		const uint32_t id;
		TimeKey(time_t t, uint32_t _id) : time(t), id(_id) {}
		bool operator< (const TimeKey& key) const
		{
			return (time == key.time ? (id < key.id) : (time < key.time));
		}
	};
	typedef map<const TimeKey, PduMonitor*> TimeMap;

	Mutex mutex;
	uint16_t msgRef;

	//SmsIdMap idMap;
	SeqNumMap seqNumMap;
	MsgRefMap msgRefMap;
	TimeMap checkTimeMap;

public:
	struct PduMonitorIterator
	{
		TimeMap::const_iterator it1;
		TimeMap::const_iterator it2;
		PduMonitorIterator(TimeMap::const_iterator i1, TimeMap::const_iterator i2)
			: it1(i1), it2(i2) {}
		virtual PduMonitor* next()
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

	void registerMonitor(PduMonitor* monitor);

	vector<PduMonitor*> getMonitors(uint16_t msgRef) const;
	ResponseMonitor* getResponseMonitor(uint32_t seqNum) const;
	DeliveryMonitor* getDeliveryMonitor(uint16_t msgRef,
		const string& serviceType) const;
	DeliveryReceiptMonitor* getDeliveryReceiptMonitor(uint16_t msgRef,
		PduData* pduData) const;
	DeliveryReceiptMonitor* getDeliveryReceiptMonitor(uint16_t msgRef,
		const string& smsId) const;
	SmeAckMonitor* getSmeAckMonitor(uint16_t msgRef) const;

	void removeMonitor(PduMonitor* monitor);

	void clear();

	PduMonitorIterator* getMonitors(time_t t1, time_t t2) const;

	int size() const;

	void dump(FILE* log) const;
};

}
}
}

#endif /* TEST_CORE_PDU_REGISTRY */
