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

/**
 * ��������� ��� �������� ������ pdu.
 */
struct PduData
{
	SMSId smsId;
	uint16_t msgRef;
	time_t submitTime;
	time_t waitTime;
	time_t validTime;
	SmppHeader* pdu;
	bool responseFlag; //���� ��������� ��������
	bool deliveryFlag; //���� ��������� ��������� ����������
	bool deliveryReceiptFlag; //���� ��������� ������������� ��������
	bool intermediateNotificationFlag; //���� ��������� ���� �����������
	PduData* replacePdu; //pdu, ������� ������ ���� �������� ������� pdu

	PduData(SmppHeader* _pdu)
		: smsId(0), msgRef(0), submitTime(0), waitTime(0), validTime(0),
		pdu(_pdu), responseFlag(false), deliveryFlag(false),
		deliveryReceiptFlag(false), intermediateNotificationFlag(false),
		replacePdu(NULL) {};

	PduData(const PduData& data)
		: smsId(data.smsId), msgRef(data.msgRef), submitTime(data.submitTime),
		waitTime(data.waitTime), validTime(data.validTime),
		pdu(data.pdu), responseFlag(data.responseFlag),
		deliveryFlag(data.deliveryFlag),
		deliveryReceiptFlag(data.deliveryReceiptFlag),
		intermediateNotificationFlag(data.intermediateNotificationFlag),
		replacePdu(data.replacePdu) {}

	~PduData()
	{
		//delete pdu;
	}

	bool complete()
	{
		return responseFlag && deliveryFlag && deliveryReceiptFlag &&
			intermediateNotificationFlag;
	}
};

/**
 * ������ ������������ pdu.
 * ��� ������ ���������������� � ������� ������� �������������.
 */
class PduRegistry
{
	typedef map<const SMSId, PduData*> SmsIdMap;
	typedef map<const uint32_t, PduData*> SeqNumMap;
	typedef map<const uint16_t, PduData*> MsgRefMap;

	struct TimeKey
	{
		time_t time;
		uint32_t seqNum;
		TimeKey(time_t t, uint32_t num) : time(t), seqNum(num) {}
		bool operator< (const TimeKey& key) const
		{
			return (time == key.time ? (seqNum < key.seqNum) : (time < key.time));
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

	PduRegistry() {}
	~PduRegistry();

	Mutex& getMutex()
	{
		return mutex;
	}

	uint16_t nextMsgRef()
	{
		return msgRef++;
	}

	/**
	 * ����������� pdu. ����������� ��������� ��������������� pdu.
	 * � ��������� ������ ����������� ������� ������ �� �������� �����.
	 */
	void putPdu(PduData& pduData);

	/**
	 * ����� ������������� pdu ��� ��������� ��� �� ������� ����������.
	 */
	PduData* getPdu(uint16_t msgRef);

	/**
	 * ����� ������������� pdu ��� ��������� �������� �� ������� �����������.
	 */
	PduData* getPdu(uint32_t seqNumber);
	
	/**
	 * ����� ������������� pdu ��� ��������� ������ ������� ���
	 * ����������� �� ������� �����������.
	 * ������������ ���:
	 * <ul>
	 * <li>SMSC Delivery Receipt.
	 * <li>Intermediate Notification.
	 * <li>SME Delivery Acknowledgement.
	 * <li>SME Manual/User Acknowledgement.
	 * </ul>
	 */
	PduData* getPdu(const SMSId smsId);

	bool removePdu(PduData* pduData);

	PduDataIterator* getPduBySubmitTime(time_t t1, time_t t2);
	
	PduDataIterator* getPduByWaitTime(time_t t1, time_t t2);

	PduDataIterator* getPduByValidTime(time_t t1, time_t t2);
};

}
}
}

#endif /* TEST_CORE_PDU_REGISTRY */
