#ifndef TEST_CORE_PDU_UTIL
#define TEST_CORE_PDU_UTIL

#include "smpp/smpp_structures.h"
#include "test/sms/SmsUtil.hpp"
#include <ctime>
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using smsc::smpp::SmppHeader;
using std::string;
using std::vector;

static const int PDU_REQUIRED_FLAG = 0x0; //pdu ���������, �� ��� �� ��������
static const int PDU_MISSING_ON_TIME_FLAG = 0x1; //���� �����, ��� PDU_REQUIRED_FLAG, ������ ����������� �� �������� �� ������
static const int PDU_RECEIVED_FLAG = 0x2; //pdu �������� �������
static const int PDU_NOT_EXPECTED_FLAG = 0x3; //������ pdu ���� �� ������

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
	time_t startTime; //������ �������� pdu
	time_t endTime; //��������� �������� pdu
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
	 * ��������� ������� ������ � ���������:
	 * <ul>
	 * <li>����� ��������� ������������ � ���������� ��������
	 * <li>��� ��������� ��� ��������� ���������
	 * <li>������������� ��� ������������� pdu
	 * </ul>
	 */
	vector<int> update(time_t recvTime, bool accepted, time_t& nextTime);

	bool isPduMissing(time_t checkTime);

	operator int() const { return flag; }
};

/**
 * ��������� ��� �������� ������ pdu.
 */
struct PduData
{
	string smsId;
	const uint16_t msgRef;
	const time_t submitTime;
	const time_t waitTime;
	const time_t validTime;
	SmppHeader* pdu;
	int responseFlag; //���� ��������� ��������
	PduReceiptFlag deliveryFlag; //���� ��������� ��������� �����������
	PduReceiptFlag deliveryReceiptFlag; //���� ��������� ������������� ��������
	int intermediateNotificationFlag; //���� ��������� ���� �����������
	PduData* replacePdu; //pdu, ������� ������ ���� �������� ������� pdu
	PduData* replacedByPdu; //pdu, ������� �������� ������� pdu

	PduData(uint16_t _msgRef, time_t _submitTime, time_t _waitTime,
		time_t _validTime, SmppHeader* _pdu, const string _smsId = "")
		: smsId(_smsId), msgRef(_msgRef), submitTime(_submitTime),
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
