#ifndef TEST_CORE_PDU_UTIL
#define TEST_CORE_PDU_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "core/synchronization/Mutex.hpp"
#include "profiler/profiler.hpp"
#include <ctime>
#include <string>
#include <map>
#include <vector>
#include <set>

namespace smsc {
namespace test {
namespace core {

using smsc::sms::Address;
using smsc::test::smpp::SmppState;
using smsc::smpp::SmppHeader;
using smsc::smpp::PduAddress;
using smsc::smpp::PduPartSm;
using smsc::smpp::PduDataPartSm;
using smsc::smpp::SmppOptional;
using smsc::smpp::SmppCommandSet::SUBMIT_SM;
using smsc::smpp::SmppCommandSet::SUBMIT_MULTI;
using smsc::smpp::SmppCommandSet::DATA_SM;
using smsc::smpp::SmppCommandSet::DELIVERY_SM;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using std::string;
using std::map;
using std::vector;
using std::set;
using std::pair;
using smsc::profiler::ProfileReportOptions::ReportNone;

typedef enum
{
	//pdu ожидается, но еще не получена
	PDU_REQUIRED_FLAG = 0x0,
	//тоже самое, что PDU_REQUIRED_FLAG, только исключается из проверок на ошибки
	//удалять после validTime + accuracy
	PDU_MISSING_ON_TIME_FLAG = 0x1,
	//если pdu получена, то обрабатывать как PDU_REQUIRED_FLAG
	//иначе как PDU_NOT_EXPECTED_FLAG
	//удалять после checkTime + accuracy
	PDU_COND_REQUIRED_FLAG = 0x2,
	//данной pdu быть не должно, не регистрировать в pdu registry
	PDU_NOT_EXPECTED_FLAG = 0x3,
} PduFlag;

typedef enum
{
	RESP_PDU_OK = 0x0, //респонс pdu отправлена со статусом ok
	RESP_PDU_ERROR = 0x1, //респонс pdu отправлена со статусом ошибки
	RESP_PDU_RESCHED = 0x2, //респонс pdu отправлена со статусом передоставки
	RESP_PDU_MISSING = 0x3, //респонс pdu не отправлена
	RESP_PDU_CONTINUE = 0x4 //продолжить доставку в текущий момент времени
} RespPduFlag;

typedef enum
{
	RESPONSE_MONITOR = 0x1,
	DELIVERY_MONITOR = 0x2,
	DELIVERY_RECEIPT_MONITOR = 0x3,
	INTERMEDIATE_NOTIFICATION_MONITOR = 0x4,
	SMS_CANCELLED_NOTIFICATION_MONITOR = 0x5,
	SME_ACK_MONITOR = 0x6,
	GENERIC_NACK_MONITOR = 0x7
} MonitorType;

typedef enum
{
	PDU_NORMAL = 1, //pdu отправляемая тестовой sme
	PDU_EXT_SME = 2, //pdu отправляемая внешней sme (db sme, profiler, ...)
	PDU_NULL_OK = 3, //pdu отправляемая sme, которая не вышлет sme ack и в deliver_sm_resp ответит ESME_ROK (smsc sme)
	PDU_NULL_ERR = 4 //pdu отправляемая sme, которая не вышлет sme ack и в deliver_sm_resp вышлет error
} PduType;

class PduDataObject
{
	int count;
	//не трогать count
	PduDataObject(const PduDataObject&) {}

public:
	PduDataObject() : count(0) {}
	virtual ~PduDataObject() {}
	//не трогать count
	PduDataObject& operator=(const PduDataObject&) { return *this; }
	void ref();
	void unref();
};

class PduData
{
	int count;
	//не трогать count
	PduData(const PduData&) {}
	PduData& operator=(const PduData&) { return *this; }
	//Mutex mutex; - вся работа с pduData внешне синхронизована

public:
	typedef map<const string, int> IntProps;
	typedef map<const string, string> StrProps;
	typedef map<const string, PduDataObject*> ObjProps;

	SmppHeader* const pdu;
	const time_t sendTime;
	set<uint32_t> checkRes;
	IntProps intProps;
	StrProps strProps;
	ObjProps objProps;
	PduData* replacePdu; //pdu, которая должна быть заменена текущей pdu
	PduData* replacedByPdu; //pdu, которая замещает текущую pdu
	
	PduData(SmppHeader* pdu, time_t sendTime, IntProps* intProps = NULL,
		StrProps* strProps = NULL, ObjProps* objProps = NULL);
	~PduData();

	void ref();
	void unref();
	string str() const;
};

/**
 * Wrapper для PduSubmitSm, PduDeliverySm и PduDataSm
 */
class SmsPduWrapper
{
	time_t sendTime;
	SmppHeader* pdu;
	PduData* pduData;
	
public:
	SmsPduWrapper(SmppHeader* pdu, time_t sendTime);
	SmsPduWrapper(PduData* pduData);
	~SmsPduWrapper() { /* не владелец submitPdu и dataPdu */ }

	bool isSubmitSm() { return (pdu->get_commandId() == SUBMIT_SM); }
	bool isSubmitMulti() { return (pdu->get_commandId() == SUBMIT_MULTI); }
	bool isDeliverSm() { return (pdu->get_commandId() == DELIVERY_SM); }
	bool isDataSm() { return (pdu->get_commandId() == DATA_SM); }
	
	const char* getServiceType();
	PduAddress& getSource();
	PduAddress& getDest();
	uint8_t getEsmClass();
	uint8_t getRegistredDelivery();
	uint8_t getDataCoding();
	time_t getWaitTime();
	time_t getValidTime();
	uint16_t getMsgRef();

	SmppHeader& get_header();
	PduPartSm& get_message();
	PduDataPartSm& get_data();
	SmppOptional& get_optional();
};

/**
 * Абстрактный базовый класс для всех типов мониторов.
 */
class PduMonitor
{
protected:
	static Mutex mutex;
	static uint32_t counter;

	uint32_t id; //внутренний уникальный номер pdu
	time_t checkTime; //время проверки получения pdu
	time_t validTime; //окончание доставки pdu
	PduFlag flag; //флаг получения pdu получателем
	int skipChecks;
	bool registered; //для контроля в pdu registry

public:
	PduData* const pduData;

	PduMonitor(time_t checkTime, time_t validTime, PduData* pduData, PduFlag flag);
	virtual ~PduMonitor();

	uint32_t getId() const { return id; }
	PduFlag getFlag() const { return flag; }
	time_t getCheckTime() const { return checkTime; }
	time_t getValidTime() const { return validTime; }
	
	void setMissingOnTime();
	void setCondRequired();
	void setNotExpected();
	
	int getSkipChecks() { return skipChecks; }
	void setSkipChecks(int val);

	bool isRegistered() const { return registered; }
	void setRegistered(bool val) { registered = val; }

	virtual MonitorType getType() const = NULL;
	virtual string str() const;

private:
	PduMonitor(const PduMonitor& monitor) { __unreachable__("Invalid"); }
	PduMonitor& operator=(const PduMonitor& monitor) { __unreachable__("Invalid"); }
};

struct ResponseMonitor : public PduMonitor
{
	uint32_t sequenceNumber;

	ResponseMonitor(uint32_t seqNum, time_t submitTime, PduData* pduData, PduFlag flag);
	virtual ~ResponseMonitor();

	virtual MonitorType getType() const { return RESPONSE_MONITOR; }
	virtual string str() const;
};

//normal sms, delivery receipt
class ReschedulePduMonitor : public PduMonitor
{
protected:
	time_t startTime; //начало доставки pdu
	time_t lastTime;
	int lastAttempt; //может быть lastAttempt = 1 для 10-ой конкатенированной sms
	int updateCount; //инкрементируется при каждом update для каждой конкатенированной sms

	void eval(time_t time, int& attempt, time_t& diff, time_t& nextTime,
		time_t& calcTime) const;

public:
	ReschedulePduMonitor(time_t startTime, time_t validTime,
		PduData* pduData, PduFlag flag);
	virtual ~ReschedulePduMonitor() {}

	time_t getLastTime() const { return lastTime; }
	time_t calcNextTime(time_t t) const;
	int getLastAttempt() const { return lastAttempt; }
	int getUpdateCount() const { return updateCount; }

	/**
	 * Проверки:
	 * <ul>
	 * <li>время получения укладывается в допустимый интервал
	 * <li>нет пропусков при повторных доставках
	 * <li>дублированные или неразрешенные pdu
	 * </ul>
	 */
	vector<int> checkSchedule(time_t recvTime) const;

	/**
	 * Обновляет текущий статус
	 */
	vector<int> update(time_t recvTime, RespPduFlag respFlag);

	virtual MonitorType getType() const = NULL;
	virtual string str() const;
};

struct DeliveryMonitor : public ReschedulePduMonitor
{
	const Address srcAddr;
	const Address destAddr;
	const string serviceType;
	const uint16_t msgRef;
	SmppState state; //статус сообщения в БД
	time_t respTime; //время репонса или время удаления сообщения
	uint32_t respStatus; //command status из delivery респонса

	DeliveryMonitor(const Address& srcAddr, const Address& destAddr,
		const string& serviceType, uint16_t msgRef, time_t waitTime,
		time_t validTime, PduData* pduData, PduFlag flag);
	virtual ~DeliveryMonitor();

	virtual MonitorType getType() const { return DELIVERY_MONITOR; }
	virtual string str() const;
};

struct DeliveryReportMonitor : public PduMonitor
{
	const uint16_t msgRef;
	SmppState state; //статус сообщения в БД
	uint32_t deliveryStatus; //command status из delivery респонса

	DeliveryReportMonitor(uint16_t msgRef, time_t checkTime, PduData* pduData,
		PduFlag flag);
	virtual ~DeliveryReportMonitor() {}

	void reschedule(time_t checkTime);
	virtual MonitorType getType() const = NULL;
	virtual string str() const;
};

struct DeliveryReceiptMonitor : public DeliveryReportMonitor
{
	DeliveryReceiptMonitor(uint16_t msgRef, time_t checkTime, PduData* pduData,
		PduFlag flag);
	virtual ~DeliveryReceiptMonitor();
	virtual MonitorType getType() const { return DELIVERY_RECEIPT_MONITOR; }
};

struct IntermediateNotificationMonitor : public DeliveryReportMonitor
{
	IntermediateNotificationMonitor(uint16_t msgRef, time_t checkTime,
		PduData* pduData, PduFlag flag);
	virtual ~IntermediateNotificationMonitor();
	virtual MonitorType getType() const { return INTERMEDIATE_NOTIFICATION_MONITOR; }
};

struct SmsCancelledNotificationMonitor : public DeliveryReportMonitor
{
	SmsCancelledNotificationMonitor(uint16_t msgRef, time_t checkTime,
		PduData* pduData, PduFlag flag);
	virtual ~SmsCancelledNotificationMonitor();
	virtual MonitorType getType() const { return SMS_CANCELLED_NOTIFICATION_MONITOR; }
};

struct SmeAckMonitor : public PduMonitor
{
	const uint16_t msgRef;
	
	SmeAckMonitor(uint16_t msgRef, time_t checkTime, PduData* pduData, PduFlag flag);
	virtual ~SmeAckMonitor();

	virtual MonitorType getType() const { return SME_ACK_MONITOR; }
	virtual string str() const;
};

struct GenericNackMonitor : public PduMonitor
{
	uint32_t sequenceNumber;
	
	GenericNackMonitor(uint32_t seqNum, time_t checkTime, PduData* pduData, PduFlag flag);
	virtual ~GenericNackMonitor();

	virtual MonitorType getType() const { return GENERIC_NACK_MONITOR; }
	virtual string str() const;
};

}
}
}

#endif /* TEST_CORE_PDU_UTIL */
