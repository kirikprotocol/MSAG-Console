#ifndef TEST_CORE_SME_PDU_REGISTRY
#define TEST_CORE_SME_PDU_REGISTRY

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include "core/synchronization/Mutex.hpp"
#include "test/sms/SmsUtil.hpp"
#include <map>

namespace smsc {
namespace test {
namespace core {

using std::map;
using smsc::sms::SMSId;
using smsc::sms::Address;
using smsc::smpp::SmppHeader;
using smsc::core::synchronization::Mutex;
using smsc::test::sms::ltAddress;

/**
 * ��������� ��� �������� ������ pdu.
 */
struct PduData
{
	SMSId smsId;
	const uint16_t msgRef;
	const time_t waitTime;
	SmppHeader* pdu;
	bool responseFlag; //���� ��������� ��������
	bool deliveryFlag; //���� ��������� ��������� ����������
	bool deliveryReceiptFlag; //���� ��������� ������������� ��������
	bool intermediateNotificationFlag; //���� ��������� ���� �����������

	PduData(uint16_t ref, const time_t time, SmppHeader* _pdu)
		: smsId(0), msgRef(ref), waitTime(time), pdu(_pdu),
		responseFlag(false), deliveryFlag(false), deliveryReceiptFlag(false),
		intermediateNotificationFlag(false) {};

	PduData(const PduData& data)
		: smsId(data.smsId), msgRef(data.msgRef), waitTime(data.waitTime),
		pdu(data.pdu), responseFlag(data.responseFlag),
		deliveryFlag(data.deliveryFlag),
		deliveryReceiptFlag(data.deliveryReceiptFlag),
		intermediateNotificationFlag(data.intermediateNotificationFlag) {}

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
	Mutex mutex;
	uint16_t msgRef;
	SmsIdMap idMap;
	SeqNumMap seqNumMap;
	MsgRefMap msgRefMap;

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

	//list<SMSId> removeExpiredPdu(time_t waitTime);
};

/**
 * ������ sme. ��� ������ ���������������� � ������� ������� �������������.
 */
class SmeRegistry
{
	typedef map<const Address, PduRegistry, ltAddress> AddressMap;
	AddressMap addrMap;

public:
	void registerSme(const Address& origAddr);
	PduRegistry* getPduRegistry(const Address& origAddr);
};

}
}
}

#endif /* TEST_CORE_SME_PDU_REGISTRY */
