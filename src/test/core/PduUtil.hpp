#ifndef TEST_CORE_PDU_UTIL
#define TEST_CORE_PDU_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "core/synchronization/Mutex.hpp"
#include <ctime>
#include <string>
#include <map>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using smsc::smpp::SmppHeader;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using std::string;
using std::map;
using std::vector;

typedef enum
{
	PDU_REQUIRED_FLAG = 0x0, //pdu ���������, �� ��� �� ��������
	PDU_MISSING_ON_TIME_FLAG = 0x1, //���� �����, ��� PDU_REQUIRED_FLAG, ������ ����������� �� �������� �� ������
	PDU_RECEIVED_FLAG = 0x2, //pdu �������� �������
	PDU_NOT_EXPECTED_FLAG = 0x3, //������ pdu ���� �� ������
	PDU_EXPIRED_FLAG = 0x4, //pdu ��������
	PDU_ERROR_FLAG = 0x5 //������ ��� �������� pdu
} PduFlag;

typedef enum
{
	RESP_PDU_OK = 0x0, //������� pdu ���������� �� �������� ok
	RESP_PDU_ERROR = 0x1, //������� pdu ���������� �� �������� ������
	RESP_PDU_RESCHED = 0x2, //������� pdu ���������� �� �������� ������������
	RESP_PDU_MISSING = 0x3 //������� pdu �� ����������
} RespPduFlag;

typedef enum
{
	RESPONSE_MONITOR = 0x1,
	DELIVERY_MONITOR = 0x2,
	DELIVERY_RECEIPT_MONITOR = 0x3,
	INTERMEDIATE_NOTIFICATION_MONITOR = 0x4,
	SME_ACK_MONITOR = 0x5
} MonitorType;

class PduDataObject
{
	int count;
	//�� ������� count
	PduDataObject(const PduDataObject&) {}

public:
	PduDataObject() : count(0) {}
	virtual ~PduDataObject() {}
	//�� ������� count
	PduDataObject& operator=(const PduDataObject&) { return *this; }
	void ref();
	void unref();
};

class PduData
{
	int count;
	//�� ������� count
	PduData(const PduData&) {}
	PduData& operator=(const PduData&) { return *this; }

public:
	typedef map<const string, int> IntProps;
	typedef map<const string, string> StrProps;
	typedef map<const string, PduDataObject*> ObjProps;

	SmppHeader* const pdu;
	const time_t submitTime;
	const uint16_t msgRef;
	string smsId;
	bool valid;
	IntProps intProps;
	StrProps strProps;
	ObjProps objProps;
	PduData* replacePdu; //pdu, ������� ������ ���� �������� ������� pdu
	PduData* replacedByPdu; //pdu, ������� �������� ������� pdu
	
	PduData(SmppHeader* pdu, time_t submitTime, uint16_t msgRef,
		IntProps* intProps = NULL, StrProps* strProps = NULL,
		ObjProps* objProps = NULL);
	~PduData();
	
	void ref();
	void unref();
};

/**
 * ����������� ������� ����� ��� ���� ����� ���������.
 */
class PduMonitor
{
protected:
	static Mutex mutex;
	static uint32_t counter;

	uint32_t id; //���������� ���������� ����� pdu
	time_t checkTime;
	time_t validTime; //��������� �������� pdu
	PduFlag flag; //���� ��������� ��������� �����������

public:
	PduData* const pduData;

	PduMonitor(time_t checkTime, time_t validTime, PduData* pduData, PduFlag flag);
	virtual ~PduMonitor();

	uint32_t getId() { return id; }
	PduFlag getFlag() { return flag; }
	time_t getCheckTime() { return checkTime; }
	time_t getValidTime() { return validTime; }
	
	void setMissingOnTime();
	void setReceived();
	void setNotExpected();
	void setExpired();
	void setError();
	virtual MonitorType getType() const = NULL;
	virtual string str() const;

private:
	PduMonitor(const PduMonitor& monitor) { __unreachable__("Invalid"); }
	PduMonitor& operator=(const PduMonitor& monitor) { __unreachable__("Invalid"); }
};

struct ResponseMonitor : public PduMonitor
{
	uint32_t sequenceNumber;

	ResponseMonitor(uint32_t seqNum, PduData* pduData, PduFlag flag);
	virtual ~ResponseMonitor();

	void setFlag(PduFlag _flag) { flag = _flag; }
	virtual MonitorType getType() const { return RESPONSE_MONITOR; }
	virtual string str() const;
};

//normal sms, delivery receipt
class ReschedulePduMonitor : public PduMonitor
{
protected:
	time_t startTime; //������ �������� pdu
	time_t lastTime;
	int lastAttempt;

	void eval(time_t time, int& attempt, time_t& diff, time_t& nextTime,
		time_t& calcTime) const;

public:
	ReschedulePduMonitor(time_t _startTime, time_t validTime, PduData* pduData,
		PduFlag flag);

	time_t getStartTime() const { return startTime; }
	time_t getLastTime() const { return lastTime; }
	time_t calcNextTime(time_t t) const;

	/**
	 * ��������:
	 * <ul>
	 * <li>����� ��������� ������������ � ���������� ��������
	 * <li>��� ��������� ��� ��������� ���������
	 * <li>������������� ��� ������������� pdu
	 * </ul>
	 */
	vector<int> checkSchedule(time_t recvTime) const;

	/**
	 * ��������� ������� ������
	 */
	vector<int> update(time_t recvTime, RespPduFlag respFlag);

	virtual MonitorType getType() const = NULL;
	virtual string str() const;
};

struct DeliveryMonitor : public ReschedulePduMonitor
{
	//uint32_t submitStatus;
	const string serviceType;

	DeliveryMonitor(const string& _serviceType, time_t waitTime,
		time_t validTime, PduData* pduData, PduFlag flag);
	virtual ~DeliveryMonitor();

	virtual MonitorType getType() const { return DELIVERY_MONITOR; }
	virtual string str() const;
};

struct DeliveryReceiptMonitor : public ReschedulePduMonitor
{
	uint8_t regDelivery; //�������� �� ������� ����������� �� ������ �������� submit_sm
	PduFlag deliveryFlag; //���� �������� ���������
	uint32_t deliveryStatus;

	DeliveryReceiptMonitor(time_t startTime, PduData* pduData, PduFlag flag);
	virtual ~DeliveryReceiptMonitor();

	void reschedule(time_t startTime);
	virtual MonitorType getType() const { return DELIVERY_RECEIPT_MONITOR; }
	virtual string str() const;
};

struct SmeAckMonitor : public PduMonitor
{
	const time_t startTime;
	SmeAckMonitor(time_t startTime, PduData* pduData, PduFlag flag);
	virtual ~SmeAckMonitor();

	virtual MonitorType getType() const { return SME_ACK_MONITOR; }
	virtual string str() const;
};

struct IntermediateNotificationMonitor : public PduMonitor
{
	uint8_t regDelivery; //�������� �� ������� ����������� �� ������ �������� submit_sm

	IntermediateNotificationMonitor(time_t startTime, PduData* pduData,
		PduFlag flag);
	virtual ~IntermediateNotificationMonitor();

	virtual MonitorType getType() const { return INTERMEDIATE_NOTIFICATION_MONITOR; }
	virtual string str() const;
};

}
}
}

#endif /* TEST_CORE_PDU_UTIL */
