#include "PduUtil.hpp"
#include "util/debug.h"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using smsc::test::conf::TestConfig;
using namespace std;

void ReschedulePduMonitor::eval(time_t time, int& attempt, time_t& diff,
	time_t& nextTime, time_t& calcTime) const
{
	__require__(time);
	__cfg_int_arr__(rescheduleTimes);
	attempt = 0;
	nextTime = startTime;
	//attempt = lastAttempt;
	//nextTime = lastTime ? lastTime : startTime;
	diff = INT_MAX;
	for (;; attempt++)
	{
		//__trace2__("eval(): nextTime = %ld", nextTime);
		time_t curDiff = time - nextTime;
		if (abs(curDiff) > abs(diff))
		{
			break;
		}
		diff = curDiff;
		calcTime = nextTime;
		if (attempt < rescheduleTimes.size())
		{
			nextTime += rescheduleTimes[attempt];
		}
		else
		{
			nextTime += rescheduleTimes[rescheduleTimes.size() - 1];
		}
		if (nextTime > validTime)
		{
			nextTime = 0;
		}
	}
	attempt--;
}

ReschedulePduMonitor::ReschedulePduMonitor(time_t _startTime, time_t validTime,
	PduData* pduData, PduFlag flag)
: PduMonitor(_startTime, validTime, pduData, flag), startTime(_startTime),
	lastTime(0), lastAttempt(0) {}

time_t ReschedulePduMonitor::calcNextTime(time_t t) const
{
	int attempt;
	time_t diff, nextTime, calcTime;
	eval(t, attempt, diff, nextTime, calcTime);
	return nextTime;
}

vector<int> ReschedulePduMonitor::checkSchedule(time_t recvTime) const
{
	vector<int> res;
	__cfg_int__(timeCheckAccuracy);
	if (recvTime < startTime)
	{
		__trace2__("check schedule: %s, recvTime = %ld is less startTime",
			str().c_str(), recvTime);
		res.push_back(1);
	}
	else if (recvTime > validTime + timeCheckAccuracy)
	{
		__trace2__("check schedule: %s, recvTime = %ld is greater validTime",
			str().c_str(), recvTime);
		res.push_back(2);
	}
	//else if (lastTime && (lastTime < startTime || lastTime > validTime))
	else
	{
		int attempt, lastAttempt = -1;
		time_t diff, lastDiff = 0;
		time_t nextTime, calcTime, lastNextTime = 0, lastCalcTime = 0;
		eval(recvTime, attempt, diff, nextTime, calcTime);
		if (lastTime)
		{
			__require__(lastTime >= startTime && lastTime <= validTime + timeCheckAccuracy);
			eval(lastTime, lastAttempt, lastDiff, lastNextTime, lastCalcTime);
		}
		__trace2__("check schedule: %s, recvTime = %ld, attempt = %d, calcTime = %ld, diff = %ld, lastAttempt = %d, lastCalcTime = %ld, lastDiff = %ld",
			str().c_str(), recvTime, attempt, calcTime, diff, lastAttempt, lastCalcTime, lastDiff);
		if (attempt - lastAttempt != 1)
		{
			res.push_back(3);
		}
		if (diff < 0)
		{
			res.push_back(4);
		}
		else if (diff > timeCheckAccuracy)
		{
			res.push_back(5);
		}
	}
	return res;
}

vector<int> ReschedulePduMonitor::update(time_t recvTime, RespPduFlag respFlag)
{
	vector<int> res;
	PduFlag prevFlag = flag;
	int attempt = 0;
	time_t diff = 0, nextTime = 0, calcTime = 0;
	switch (flag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			{
				eval(recvTime, attempt, diff, nextTime, calcTime);
				lastAttempt = attempt;
				switch (respFlag)
				{
					case RESP_PDU_OK:
					case RESP_PDU_ERROR:
						setReceived();
						lastTime = recvTime;
						break;
					case RESP_PDU_RESCHED:
					case RESP_PDU_MISSING:
						if (!nextTime || nextTime > validTime)
						{
							setNotExpected();
						}
						else
						{
							checkTime = nextTime;
						}
						lastTime = recvTime;
						break;
					/*
					case RESP_PDU_MISSING:
						//lastTime � nextTime � ������ 8 ���������� timeout
						if (!nextTime || nextTime + 8 > validTime)
						{
							flag = PDU_RECEIVED_FLAG;
						}
						lastTime = recvTime + 8;
						break;
					*/
					default:
						__unreachable__("Unknown resp flag");
				}
			}
			break;
		case PDU_RECEIVED_FLAG:
			res.push_back(1);
			break;
		case PDU_NOT_EXPECTED_FLAG:
			res.push_back(2);
			break;
		default:
			__unreachable__("Unknown flag");
	}
	__trace2__("update monitor: %s, recvTime = %ld, respFlag = %d, flag: %d -> %d, attempt = %d, calcTime = %ld, diff = %ld",
		str().c_str(), recvTime, respFlag, flag, prevFlag, attempt, calcTime, diff);
	return res;
}

Mutex PduMonitor::mutex = Mutex();
uint32_t PduMonitor::counter = 1;

PduData::PduData(SmppHeader* _pdu, time_t _submitTime, uint16_t _msgRef,
	IntProps* _intProps, StrProps* _strProps, ObjProps* _objProps)
: pdu(_pdu), submitTime(_submitTime), msgRef(_msgRef), valid(false), count(0),
	replacePdu(NULL), replacedByPdu(NULL)
{
	__require__(pdu);
	if (_intProps)
	{
		intProps = *_intProps;
	}
	if (_strProps)
	{
		strProps = *_strProps;
	}
	if (_objProps)
	{
		objProps = *_objProps;
	}
	__trace2__("PduData created = %p", this);
}

PduData::~PduData()
{
	//��������� ����� � ���������� � ���������� pdu
	if (replacePdu)
	{
		replacePdu->replacedByPdu = NULL;
	}
	if (replacedByPdu)
	{
		replacedByPdu->replacePdu = NULL;
	}
	if (!count)
	{
		__require__(pdu);
		disposePdu(pdu);
	}
	__trace2__("PduData deleted = %p", this);
}

void PduData::ref()
{
	count++;
}
void PduData::unref()
{
	count--;
	if (!count)
	{
		delete this;
	}
}

PduMonitor::PduMonitor(time_t _checkTime, time_t _validTime,
	PduData* _pduData, PduFlag _flag)
: checkTime(_checkTime), validTime(_validTime), pduData(_pduData), flag(_flag)
{
	__require__(pduData);
	pduData->ref();
	if (flag != PDU_REQUIRED_FLAG)
	{
		checkTime = validTime;
	}
	MutexGuard mguard(mutex);
	id = counter++;
}
	
PduMonitor::~PduMonitor()
{
	__require__(pduData);
	pduData->unref();
}

void PduMonitor::setMissingOnTime()
{
	flag = PDU_MISSING_ON_TIME_FLAG;
	checkTime = validTime;
	__trace2__("monitor set missing: %s", str().c_str());
}
	
void PduMonitor::setReceived()
{
	flag = PDU_RECEIVED_FLAG;
	checkTime = validTime;
	__trace2__("monitor set received: %s", str().c_str());
}

void PduMonitor::setNotExpected()
{
	flag = PDU_NOT_EXPECTED_FLAG;
	checkTime = validTime;
	__trace2__("monitor set not expected: %s", str().c_str());
}

string PduMonitor::str() const
{
	ostringstream s;
	switch (getType())
	{
		case RESPONSE_MONITOR:
			s << "type = response";
			break;
		case DELIVERY_MONITOR:
			s << "type = delivery";
			break;
		case DELIVERY_RECEIPT_MONITOR:
			s << "type = delivery receipt";
			break;
		case INTERMEDIATE_NOTIFICATION_MONITOR:
			s << "type = intermediate notification";
			break;
		case SME_ACK_MONITOR:
			s << "type = sme ack";
			break;
		default:
			__unreachable__("Invalid monitor type");
	}
	switch (flag)
	{
		case PDU_REQUIRED_FLAG:
			s << ", flag = required";
			break;
		case PDU_MISSING_ON_TIME_FLAG:
			s << ", flag = missing";
			break;
		case PDU_RECEIVED_FLAG:
			s << ", flag = received";
			break;
		case PDU_NOT_EXPECTED_FLAG:
			s << ", flag = not expected";
			break;
		default:
			__unreachable__("Invalid pdu flag");
	}
	s << ", pduData = " << (void*) pduData;
	s << ", valid = " << (pduData->valid ? "true" : "false");
	s << ", checkTime = " << checkTime << ", validTime = " << validTime;
	return s.str();
}

ResponseMonitor::ResponseMonitor(uint32_t seqNum, PduData* pduData, PduFlag flag)
: PduMonitor(pduData->submitTime, pduData->submitTime, pduData, flag),
	sequenceNumber(seqNum)
{
	__trace2__("monitor created: %s", str().c_str());
}

ResponseMonitor::~ResponseMonitor()
{
	__trace2__("monitor deleted: %s", str().c_str());
}

string ResponseMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", sequenceNumber = " << sequenceNumber;
	return s.str();
}

string ReschedulePduMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", startTime = " << startTime <<
		", lastTime = " << lastTime << ", lastAttempt = " << lastAttempt;
	return s.str();
}

DeliveryMonitor::DeliveryMonitor(const string& _serviceType, time_t waitTime,
	time_t validTime, PduData* pduData, PduFlag flag)
: ReschedulePduMonitor(waitTime, validTime, pduData, flag),
	serviceType(_serviceType)
{
	__trace2__("monitor created: %s", str().c_str());
}

DeliveryMonitor::~DeliveryMonitor()
{
	__trace2__("monitor deleted: %s", str().c_str());
}

string DeliveryMonitor::str() const
{
	ostringstream s;
	s << ReschedulePduMonitor::str() << ", serviceType = " << serviceType;
	return s.str();
}

DeliveryReceiptMonitor::DeliveryReceiptMonitor(time_t startTime,
	PduData* pduData, PduFlag flag)
: ReschedulePduMonitor(startTime, 0, pduData, flag), regDelivery(0xff),
	deliveryFlag(PDU_REQUIRED_FLAG), deliveryStatus(0)
{
	__cfg_int__(maxValidPeriod);
	validTime = startTime + maxValidPeriod;
	if (flag != PDU_REQUIRED_FLAG)
	{
		checkTime = validTime;
	}
	__trace2__("monitor created: %s", str().c_str());
}

DeliveryReceiptMonitor::~DeliveryReceiptMonitor()
{
	__trace2__("monitor deleted: %s", str().c_str());
}

void DeliveryReceiptMonitor::reschedule(time_t _startTime)
{
	//���������� ������ ���� pdu �� ���� �� ���� ��������
	__require__(!lastTime && !lastAttempt);
	__cfg_int__(maxValidPeriod);
	startTime = _startTime;
	validTime = startTime + maxValidPeriod;
}

string DeliveryReceiptMonitor::str() const
{
	ostringstream s;
	s << ReschedulePduMonitor::str() << ", regDelivery = " << (int) regDelivery <<
		", deliveryFlag = " << (int) deliveryFlag << ", deliveryStatus = " <<
		deliveryStatus;
	return s.str();
}

SmeAckMonitor::SmeAckMonitor(time_t _startTime, PduData* pduData, PduFlag flag)
: PduMonitor(_startTime, 0, pduData, flag), startTime(_startTime)
{
	__cfg_int__(maxValidPeriod);
	validTime = startTime + maxValidPeriod;
	if (flag != PDU_REQUIRED_FLAG)
	{
		checkTime = validTime;
	}
	__trace2__("monitor created: %s", str().c_str());
}

SmeAckMonitor::~SmeAckMonitor()
{
	__trace2__("monitor deleted: %s", str().c_str());
}

string SmeAckMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", startTime = " << startTime;
	return s.str();
}

IntermediateNotificationMonitor::IntermediateNotificationMonitor(
	time_t startTime, PduData* pduData, PduFlag flag)
: PduMonitor(startTime, 0, pduData, flag), regDelivery(0xff)
{
	__cfg_int__(maxValidPeriod);
	validTime = startTime + maxValidPeriod;
	if (flag != PDU_REQUIRED_FLAG)
	{
		checkTime = validTime;
	}
	__trace2__("monitor created: %s", str().c_str());
}

IntermediateNotificationMonitor::~IntermediateNotificationMonitor()
{
	__trace2__("monitor deleted: %s", str().c_str());
}

string IntermediateNotificationMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", regDelivery = " << (int) regDelivery;
	return s.str();
}

}
}
}

