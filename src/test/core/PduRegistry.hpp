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
	time_t waitTime;
	SmppHeader* pdu;
	bool responseFlag; //���� ��������� ��������
	bool deliveryFlag; //���� ��������� ��������� ����������
	bool deliveryReceiptFlag; //���� ��������� ������������� ��������
	bool intermediateNotificationFlag; //���� ��������� ���� �����������
	PduData* replacePdu; //pdu, ������� ������ ���� �������� ������� pdu

	PduData(SmppHeader* _pdu)
		: smsId(0), msgRef(0), waitTime(0), pdu(_pdu), responseFlag(false),
		deliveryFlag(false), deliveryReceiptFlag(false),
		intermediateNotificationFlag(false), replacePdu(NULL) {};

	PduData(const PduData& data)
		: smsId(data.smsId), msgRef(data.msgRef), waitTime(data.waitTime),
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
	typedef multimap<const time_t, PduData*> WaitTimeMap;
	typedef pair<const time_t, PduData*> WaitTimeMapPair;
	Mutex mutex;
	uint16_t msgRef;
	SmsIdMap idMap;
	SeqNumMap seqNumMap;
	MsgRefMap msgRefMap;
	WaitTimeMap waitTimeMap;

public:
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

	bool removePdu(const PduData& pduData);

	vector<PduData*> getOverduePdu(time_t waitTime);

	PduData* getFirstPendingSubmitSmPdu(time_t waitTime);
};

}
}
}

#endif /* TEST_CORE_PDU_REGISTRY */
