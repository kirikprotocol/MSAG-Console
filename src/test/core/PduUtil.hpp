#ifndef TEST_CORE_PDU_UTIL
#define TEST_CORE_PDU_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"
#include "test/TestConfig.hpp"
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
using namespace smsc::test;

typedef enum
{
	PDU_REQUIRED_FLAG = 0x0, //pdu ожидаетс€, но еще не получена
	PDU_MISSING_ON_TIME_FLAG = 0x1, //тоже самое, что PDU_REQUIRED_FLAG, только исключаетс€ из проверок на ошибки
	PDU_RECEIVED_FLAG = 0x2, //pdu получена воврем€
	PDU_NOT_EXPECTED_FLAG = 0x3 //данной pdu быть не должно
} PduFlag;

typedef enum
{
	RESP_PDU_OK = 0x0, //респонс pdu отправлена со статусом ok
	RESP_PDU_ERROR = 0x1, //респонс pdu отправлена со статусом ошибки
	RESP_PDU_RESCHED = 0x2, //респонс pdu отправлена со статусом передоставки
	RESP_PDU_MISSING = 0x3 //респонс pdu не отправлена
} RespPduFlag;

//normal sms, delivery receipt
class PduReceiptFlag
{
	PduFlag flag;
	time_t startTime; //начало доставки pdu
	time_t endTime; //окончание доставки pdu
	time_t lastTime;
	int lastAttempt;

	void eval(time_t time, int& attempt, time_t& diff, time_t& nextTime,
		time_t& calcTime) const;

public:
	PduReceiptFlag(PduFlag flg, time_t start, time_t end) :
		flag(flg), startTime(start), endTime(end), lastTime(0), lastAttempt(0)
	{
		//__require__(startTime <= endTime);
	}

	PduReceiptFlag& operator= (PduFlag _flag) { flag = _flag; }
	
	bool operator== (PduFlag _flag) { return (flag == _flag); }
	bool operator!= (PduFlag _flag) { return (flag != _flag); }

	time_t getNextTime(time_t t) const;

	/**
	 * ѕроверки:
	 * <ul>
	 * <li>врем€ получени€ укладываетс€ в допустимый интервал
	 * <li>нет пропусков при повторных доставках
	 * <li>дублированные или неразрешенные pdu
	 * </ul>
	 */
	vector<int> checkSchedule(time_t recvTime) const;

	/**
	 * ќбновл€ет текущий статус
	 */
	vector<int> update(time_t recvTime, RespPduFlag respFlag);

	bool isPduMissing(time_t checkTime) const;

	operator PduFlag() const { return flag; }
};

/**
 * —труктура дл€ хранени€ данных pdu.
 */
struct PduData
{
	typedef map<const string, int> IntProps;
	typedef map<const string, string> StrProps;

	static Mutex mutex;
	static uint32_t counter;
	uint32_t id; //внутренний уникальный номер pdu
	string smsId;
	const uint16_t msgRef;
	const time_t submitTime;
	const time_t waitTime;
	const time_t validTime;
	SmppHeader* pdu;
	uint32_t submitStatus;
	uint32_t deliveryStatus;
	int reportOptions; //значение из профил€ отправител€ на момент отправки submit_sm
	int responseFlag; //флаг получени€ респонса
	PduReceiptFlag deliveryFlag; //флаг получени€ сообщени€ получателем
	PduReceiptFlag deliveryReceiptFlag; //флаг получени€ подтверждени€ доставки
	int intermediateNotificationFlag; //флаг получени€ всех нотификаций
	PduData* replacePdu; //pdu, котора€ должна быть заменена текущей pdu
	PduData* replacedByPdu; //pdu, котора€ замещает текущую pdu
	map<const string, int> intProps;
	map<const string, string> strProps;

	PduData(uint16_t _msgRef, time_t _submitTime, time_t _waitTime,
		time_t _validTime, SmppHeader* _pdu, const string _smsId = "")
		: smsId(_smsId), msgRef(_msgRef), submitTime(_submitTime),
		waitTime(_waitTime), validTime(_validTime), pdu(_pdu),
		submitStatus(0), deliveryStatus(0), reportOptions(-1),
		responseFlag(PDU_REQUIRED_FLAG),
		deliveryFlag(PDU_REQUIRED_FLAG, waitTime, validTime),
		deliveryReceiptFlag(PDU_REQUIRED_FLAG, waitTime, validTime),
		intermediateNotificationFlag(PDU_REQUIRED_FLAG),
		replacePdu(NULL), replacedByPdu(NULL)
	{
		MutexGuard mguard(mutex);
		id = counter++;
	}

	~PduData()
	{
		//disposePdu(pdu);
	}

	bool complete()
	{
		return responseFlag && ((int) deliveryFlag) &&
			((int) deliveryReceiptFlag) && ((int) intermediateNotificationFlag);
	}
};

}
}
}

#endif /* TEST_CORE_PDU_UTIL */
