#ifndef TEST_CORE_PDU_UTIL
#define TEST_CORE_PDU_UTIL

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include <ctime>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using smsc::sms::SMSId;
using smsc::smpp::SmppHeader;
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

	time_t eval(time_t time, int& attempt, time_t& diff);

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

	/**
	 * Обновляет текущий статус и проверяет:
	 * <ul>
	 * <li>время получения укладывается в допустимый интервал
	 * <li>нет пропусков при повторных доставках
	 * <li>дублированные или неразрешенные pdu
	 * </ul>
	 */
	vector<int> update(time_t recvTime, bool accepted, time_t& nextTime);

	bool isPduMissing(time_t checkTime);

	operator int() const { return flag; }
};

/**
 * Структура для хранения данных pdu.
 */
struct PduData
{
	SMSId smsId;
	const uint16_t msgRef;
	const time_t submitTime;
	const time_t waitTime;
	const time_t validTime;
	SmppHeader* pdu;
	int responseFlag; //флаг получения респонса
	PduReceiptFlag deliveryFlag; //флаг получения сообщения плучателем
	PduReceiptFlag deliveryReceiptFlag; //флаг получения подтверждения доставки
	int intermediateNotificationFlag; //флаг получения всех нотификаций
	PduData* replacePdu; //pdu, которая должна быть заменена текущей pdu
	PduData* replacedByPdu; //pdu, которая замещает текущую pdu

	PduData(uint16_t _msgRef, time_t _submitTime, time_t _waitTime,
		time_t _validTime, SmppHeader* _pdu)
		: smsId(0), msgRef(_msgRef), submitTime(_submitTime),
		waitTime(_waitTime), validTime(_validTime), pdu(_pdu),
		responseFlag(PDU_REQUIRED_FLAG),
		deliveryFlag(PDU_REQUIRED_FLAG, waitTime, validTime),
		deliveryReceiptFlag(PDU_REQUIRED_FLAG, waitTime, validTime),
		intermediateNotificationFlag(PDU_REQUIRED_FLAG),
		replacePdu(NULL), replacedByPdu(NULL) {}

	PduData(const PduData& data)
		: smsId(data.smsId), msgRef(data.msgRef), submitTime(data.submitTime),
		waitTime(data.waitTime), validTime(data.validTime),
		pdu(data.pdu), responseFlag(data.responseFlag),
		deliveryFlag(data.deliveryFlag),
		deliveryReceiptFlag(data.deliveryReceiptFlag),
		intermediateNotificationFlag(data.intermediateNotificationFlag),
		replacePdu(data.replacePdu), replacedByPdu(data.replacedByPdu) {}

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
