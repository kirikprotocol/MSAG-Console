#ifndef TEST_CORE_PDU_UTIL
#define TEST_CORE_PDU_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"
#include "core/synchronization/Mutex.hpp"
#include <ctime>
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using smsc::smpp::SmppHeader;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using std::string;
using std::vector;

static const int PDU_REQUIRED_FLAG = 0x0; //pdu ожидается, но еще не получена
static const int PDU_MISSING_ON_TIME_FLAG = 0x1; //тоже самое, что PDU_REQUIRED_FLAG, только исключается из проверок на ошибки
static const int PDU_RECEIVED_FLAG = 0x2; //pdu получена вовремя
static const int PDU_NOT_EXPECTED_FLAG = 0x3; //данной pdu быть не должно

class DeliveryIterator
{
	const time_t startTime;
	const time_t endTime;
	int attempt;
	time_t time;
public:
	DeliveryIterator(time_t start, time_t end)
		: startTime(start), endTime(end), attempt(0), time(start) {}
	DeliveryIterator& operator++();
	DeliveryIterator& operator++(int) { return operator++(); }
	time_t getTime() { return time; }
	int getAttempt() { return attempt; }
};

//normal sms, delivery receipt
class PduReceiptFlag
{
	int flag;
	time_t startTime; //начало доставки pdu
	time_t endTime; //окончание доставки pdu
	time_t lastTime;

	time_t eval(time_t time, int& attempt, time_t& diff) const;

public:
	PduReceiptFlag(int flg, time_t start, time_t end) :
		flag(flg), startTime(start), endTime(end), lastTime(0) {}

	PduReceiptFlag(const PduReceiptFlag& f) : flag(f.flag),
		startTime(f.startTime), endTime(f.endTime), lastTime(f.lastTime) {}

	PduReceiptFlag& operator= (int _flag) { flag = _flag; }
	PduReceiptFlag& operator= (const PduReceiptFlag& f)
	{ flag = f.flag; startTime = f.startTime; endTime = f.endTime; lastTime = f.lastTime; }
	
	bool operator== (int _flag) { return (flag == _flag); }
	bool operator!= (int _flag) { return (flag != _flag); }

	time_t getNextTime(time_t t) const;

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
	vector<int> update(time_t recvTime, bool accepted);

	bool isPduMissing(time_t checkTime) const;

	operator int() const { return flag; }
};

/**
 * Структура для хранения данных pdu.
 */
struct PduData
{
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
	int responseFlag; //флаг получения респонса
	PduReceiptFlag deliveryFlag; //флаг получения сообщения получателем
	PduReceiptFlag deliveryReceiptFlag; //флаг получения подтверждения доставки
	int intermediateNotificationFlag; //флаг получения всех нотификаций
	PduData* replacePdu; //pdu, которая должна быть заменена текущей pdu
	PduData* replacedByPdu; //pdu, которая замещает текущую pdu
	//существуют другие pdu с такими же source_addr, //dest_addr, service_type,
	//созданные с replace_if_present_flag = 0
	bool hasSmppDuplicates;

	PduData(uint16_t _msgRef, time_t _submitTime, time_t _waitTime,
		time_t _validTime, SmppHeader* _pdu, const string _smsId = "")
		: smsId(_smsId), msgRef(_msgRef), submitTime(_submitTime),
		waitTime(_waitTime), validTime(_validTime), pdu(_pdu),
		submitStatus(0), deliveryStatus(0), responseFlag(PDU_REQUIRED_FLAG),
		deliveryFlag(PDU_REQUIRED_FLAG, waitTime, validTime),
		deliveryReceiptFlag(PDU_REQUIRED_FLAG, waitTime, validTime),
		intermediateNotificationFlag(PDU_REQUIRED_FLAG),
		replacePdu(NULL), replacedByPdu(NULL), hasSmppDuplicates(false)
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
