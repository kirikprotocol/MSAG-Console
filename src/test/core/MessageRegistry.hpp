#ifndef TEST_CORE_MESSAGE_REGISTRY
#define TEST_CORE_MESSAGE_REGISTRY

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include "core/synchronization/Mutex.hpp"
#include "test/sms/SmsUtil.hpp"
#include <map>
#include <list>

namespace smsc {
namespace test {
namespace core {

using std::map;
using std::list;
using smsc::sms::SMSId;
using smsc::sms::SMS;
using smsc::sms::Address;
using smsc::smpp::SmppHeader;
using smsc::core::synchronization::Mutex;
using smsc::test::sms::ltAddress;

/**
 * ��������� ��� �������� ������ sms ��� pdu.
 */
struct MsgData
{
	const SMSId smsId;
	const uint16_t msgRef;
	const time_t waitTime;
	const SMS* sms;
	const SmppHeader* pdu;
	bool deliveryFlag;
	bool deliveryReceiptFlag;
	bool intermediateNotificationFlag;

	MsgData(const SMSId id, uint16_t ref, const time_t time, const SMS* _sms,
		const SmppHeader* _pdu)
		: smsId(id), msgRef(ref), waitTime(time), sms(_sms), pdu(_pdu) {};

	~MsgData()
	{
		if (sms) delete sms;
		if (pdu) delete pdu;
	}

	bool complete()
	{
		return deliveryFlag && deliveryReceiptFlag && intermediateNotificationFlag;
	}
};

/**
 * ������ ������������ sms ��� pdu.
 * ��� ������ ���������������� � ������� ������� �������������.
 */
class MessageRegistry
{
public:
	Mutex& getMutex(const Address& origAddr);

	uint16_t nextMsgRef(const Address& origAddr);

	/*
	 * ����������� ��������� (sms ��� pdu).
	 */
	void putMsg(const Address& origAddr, MsgData& msgData);

	/*
	 * ����� ������������� ��������� ��� ��������� ��� �� ������� ����������.
	 */
	MsgData* getMsg(const Address& origAddr, uint16_t msgRef);

	/**
	 * ����� ������������� ��������� ��� ��������� ������ ������� ���
	 * ����������� �� ������� �����������.
	 * ������������ ���:
	 * <ul>
	 * <li>SMSC Delivery Receipt.
	 * <li>Intermediate Notification.
	 * <li>SME Delivery Acknowledgement.
	 * <li>SME Manual/User Acknowledgement.
	 * </ul>
	 */
	MsgData* getMsg(const Address& origAddr, const SMSId smsId);

	bool removeMsg(const Address& origAddr, const SMSId smsId);

	list<SMSId> deleteExpiredMsg(const Address& origAddr, time_t waitTime);

private:
	typedef list<MsgData> MsgList;
	struct AddressData
	{
		Mutex mutex;
		uint16_t msgRef;
		MsgList messages;
	};
	typedef map<const Address, AddressData, ltAddress> AddressMap;
	AddressMap addrMap;
};

}
}
}

#endif /* TEST_CORE_MESSAGE_REGISTRY */
