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
	struct SeqNumKey
	{
		const uint32_t seqNum;
		const MonitorType type;
		SeqNumKey(uint32_t _seqNum, MonitorType _type)
		: seqNum(_seqNum), type(_type) {}
		bool operator< (const SeqNumKey& key) const
		{
			if (seqNum != key.seqNum) { return (seqNum < key.seqNum); }
			return type < key.type;
		}
	};
	typedef map<const SeqNumKey, PduMonitor*> SeqNumMap;

	struct MsgRefKey
	{
		const uint16_t msgRef;
		const MonitorType type;
		const uint32_t id;
		MsgRefKey(uint16_t _msgRef, MonitorType _type, uint32_t _id)
		: msgRef(_msgRef), type(_type), id(_id) {}
		bool operator< (const MsgRefKey& key) const
		{
			if (msgRef != key.msgRef) { return (msgRef < key.msgRef); }
			if (type != key.type) { return (type < key.type); }
			return id < key.id;
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
			if (time != key.time) { return (time < key.time); }
			return (id < key.id);
		}
	};
	typedef map<const TimeKey, PduMonitor*> TimeMap;

	Mutex mutex;

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

	PduRegistry() {}
	~PduRegistry();

	Mutex& getMutex()
	{
		return mutex;
	}

	uint16_t nextMsgRef()
	{
		static Mutex m;
		static uint16_t msgRef = 0;
		MutexGuard guard(m);
		return ++msgRef;
	}

	void registerMonitor(PduMonitor* monitor);

	ResponseMonitor* getResponseMonitor(uint32_t seqNum) const;
	DeliveryMonitor* getDeliveryMonitor(uint16_t msgRef) const;
	DeliveryReceiptMonitor* getDeliveryReceiptMonitor(uint16_t msgRef) const;
	SmeAckMonitor* getSmeAckMonitor(uint16_t msgRef) const;

	void removeMonitor(PduMonitor* monitor);

	void clear();

	PduMonitorIterator* getMonitors(time_t t1, time_t t2) const;

	int size() const;

	void dump(FILE* log) const;

protected:
	PduMonitor* getMonitor(uint16_t msgRef, MonitorType type) const;
	PduMonitor* getMonitor(uint32_t seqNum, MonitorType type) const;
	void registerMonitor(uint32_t seqNum, PduMonitor* monitor);
	void registerMonitor(uint16_t msgRef, PduMonitor* monitor);
	void registerMonitor(time_t t, PduMonitor* monitor);
};

}
}
}

#endif /* TEST_CORE_PDU_REGISTRY */
