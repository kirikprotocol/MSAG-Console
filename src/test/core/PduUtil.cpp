#include "PduUtil.hpp"
#include "test/util/Util.hpp"
#include "util/debug.h"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using smsc::test::conf::TestConfig;
using smsc::test::util::operator<<;
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
		__trace2__("check schedule: recvTime = %ld is less startTime, monitor = %s",
			recvTime, str().c_str());
		res.push_back(1);
	}
	else if (recvTime > validTime + timeCheckAccuracy)
	{
		__trace2__("check schedule: recvTime = %ld is greater validTime, monitor = %s",
			recvTime, str().c_str());
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
		__trace2__("check schedule: recvTime = %ld, attempt = %d, calcTime = %ld, diff = %ld, lastAttempt = %d, lastCalcTime = %ld, lastDiff = %ld, monitor = %s",
			recvTime, attempt, calcTime, diff, lastAttempt, lastCalcTime, lastDiff, str().c_str());
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
	__require__(!registered);
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
						setReceived();
						break;
					case RESP_PDU_ERROR:
						setError();
						break;
					case RESP_PDU_RESCHED:
					case RESP_PDU_MISSING:
						if (!nextTime || nextTime > validTime)
						{
							setExpired();
						}
						else
						{
							checkTime = nextTime;
						}
						break;
					default:
						__unreachable__("Unknown resp flag");
				}
				lastTime = recvTime;
			}
			break;
		case PDU_RECEIVED_FLAG:
			res.push_back(1);
			break;
		case PDU_NOT_EXPECTED_FLAG:
			res.push_back(2);
			break;
		case PDU_EXPIRED_FLAG:
			res.push_back(3);
			break;
		case PDU_ERROR_FLAG:
			res.push_back(4);
			break;
		default:
			__unreachable__("Unknown flag");
	}
	__trace2__("update monitor: recvTime = %ld, respFlag = %d, flag: %d -> %d, attempt = %d, calcTime = %ld, diff = %ld, monitor = %s",
		recvTime, respFlag, prevFlag, flag, attempt, calcTime, diff, str().c_str());
	return res;
}

Mutex PduMonitor::mutex = Mutex();
uint32_t PduMonitor::counter = 1;

void PduDataObject::ref()
{
	count++;
	//__trace2__("PduDataObject::ref(): this = %p, count = %d", this, count);
}
void PduDataObject::unref()
{
	//__trace2__("PduDataObject::unref(): this = %p, count = %d", this, count);
	__require__(count > 0);
	count--;
	if (!count)
	{
		delete this;
	}
}

PduData::PduData(SmppHeader* _pdu, time_t _submitTime, uint16_t _msgRef,
	int _reportOptions, IntProps* _intProps,
	StrProps* _strProps, ObjProps* _objProps)
: pdu(_pdu), submitTime(_submitTime), msgRef(_msgRef),
	reportOptions(_reportOptions), valid(false), count(0),
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
	//разорвать связь с замещающей и замещаемой pdu
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
	for (ObjProps::iterator it = objProps.begin(); it != objProps.end(); it++)
	{
		__require__(it->second);
		it->second->unref();
	}
	__trace2__("PduData deleted = %p", this);
}

void PduData::ref()
{
	//__trace2__("PduData::ref(): this = %p, count = %d", this, count);
	count++;
}
void PduData::unref()
{
	//__trace2__("PduData::unref(): this = %p, count = %d", this, count);
	__require__(count > 0);
	count--;
	if (!count)
	{
		delete this;
	}
}

string PduData::str() const
{
	ostringstream s;
	s << "this = " << (void*) this;
	s << ", pdu = " << (void*) pdu;
	s << ", msgRef = " << msgRef;
	s << ", smsId = " << smsId;
	s << ", reportOptions = " << reportOptions;
	s << ", submitTime = " << submitTime;
	s << ", valid = " << (valid ? "true" : "false");
	s << ", replacePdu = " << (void*) replacePdu;
	s << ", replacedByPdu = " << (void*) replacedByPdu;
	s << ", count = " << count;
	return s.str();
}

PduMonitor::PduMonitor(time_t _checkTime, time_t _validTime,
	PduData* _pduData, PduFlag _flag)
: checkTime(_checkTime), validTime(_validTime), pduData(_pduData), flag(_flag),
	registered(false)
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
	__require__(!registered);
	__require__(pduData);
	pduData->unref();
}

void PduMonitor::setMissingOnTime()
{
	__require__(!registered);
	flag = PDU_MISSING_ON_TIME_FLAG;
	checkTime = validTime;
	__trace2__("monitor set missing: %s", str().c_str());
}
	
void PduMonitor::setReceived()
{
	__require__(!registered);
	flag = PDU_RECEIVED_FLAG;
	checkTime = validTime;
	__trace2__("monitor set received: %s", str().c_str());
}

void PduMonitor::setNotExpected()
{
	__require__(!registered);
	flag = PDU_NOT_EXPECTED_FLAG;
	checkTime = validTime;
	__trace2__("monitor set not expected: %s", str().c_str());
}

void PduMonitor::setExpired()
{
	__require__(!registered);
	flag = PDU_EXPIRED_FLAG;
	checkTime = validTime;
	__trace2__("monitor set expired: %s", str().c_str());
}

void PduMonitor::setError()
{
	__require__(!registered);
	flag = PDU_ERROR_FLAG;
	checkTime = validTime;
	__trace2__("monitor set error: %s", str().c_str());
}

string PduMonitor::str() const
{
	ostringstream s;
	s << "id = " << id;
	switch (getType())
	{
		case RESPONSE_MONITOR:
			s << ", type = response";
			break;
		case DELIVERY_MONITOR:
			s << ", type = delivery";
			break;
		case DELIVERY_RECEIPT_MONITOR:
			s << ", type = delivery receipt";
			break;
		case INTERMEDIATE_NOTIFICATION_MONITOR:
			s << ", type = intermediate notification";
			break;
		case SME_ACK_MONITOR:
			s << ", type = sme ack";
			break;
		case GENERIC_NACK_MONITOR:
			s << ", type = generic nack";
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
		case PDU_EXPIRED_FLAG:
			s << ", flag = expired";
			break;
		case PDU_ERROR_FLAG:
			s << ", flag = error";
			break;
		default:
			__unreachable__("Invalid pdu flag");
	}
	s << ", checkTime = " << checkTime << ", validTime = " << validTime;
	s << ", pduData = {" << pduData->str() << "}";
	return s.str();
}

ResponseMonitor::ResponseMonitor(uint32_t seqNum, PduData* pduData, PduFlag flag)
: PduMonitor(pduData->submitTime, pduData->submitTime, pduData, flag),
	sequenceNumber(seqNum)
{
	//__trace2__("monitor created: %s", str().c_str());
}

ResponseMonitor::~ResponseMonitor()
{
	//__trace2__("monitor deleted: %s", str().c_str());
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
	//__trace2__("monitor created: %s", str().c_str());
}

DeliveryMonitor::~DeliveryMonitor()
{
	//__trace2__("monitor deleted: %s", str().c_str());
}

string DeliveryMonitor::str() const
{
	ostringstream s;
	s << ReschedulePduMonitor::str() << ", serviceType = " << serviceType;
	return s.str();
}

DeliveryReportMonitor::DeliveryReportMonitor(time_t _checkTime,
	PduData* _pduData, PduFlag _flag)
: PduMonitor(_checkTime, 0, _pduData, _flag), deliveryFlag(PDU_REQUIRED_FLAG),
	deliveryStatus(0)
{
	__cfg_int__(maxValidPeriod);
	validTime = checkTime + maxValidPeriod;
	if (flag != PDU_REQUIRED_FLAG)
	{
		checkTime = validTime;
	}
}

void DeliveryReportMonitor::reschedule(time_t _checkTime)
{
	__require__(!registered);
	__cfg_int__(maxValidPeriod);
	switch (flag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			checkTime = _checkTime;
			validTime = checkTime + maxValidPeriod;
			flag = PDU_REQUIRED_FLAG;
			break;
		default:
			__unreachable__("Invalid flag");
	}
}

string DeliveryReportMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", deliveryFlag = " << deliveryFlag <<
		", deliveryStatus = " << deliveryStatus;
	return s.str();
}

DeliveryReceiptMonitor::DeliveryReceiptMonitor(time_t checkTime, PduData* pduData,
	PduFlag flag) : DeliveryReportMonitor(checkTime, pduData, flag)
{
	//__trace2__("monitor created: %s", str().c_str());
}

DeliveryReceiptMonitor::~DeliveryReceiptMonitor()
{
	//__trace2__("monitor deleted: %s", str().c_str());
}

IntermediateNotificationMonitor::IntermediateNotificationMonitor(time_t checkTime,
	PduData* pduData, PduFlag flag)
: DeliveryReportMonitor(checkTime, pduData, flag)
{
	//__trace2__("monitor created: %s", str().c_str());
}

IntermediateNotificationMonitor::~IntermediateNotificationMonitor()
{
	//__trace2__("monitor deleted: %s", str().c_str());
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
	//__trace2__("monitor created: %s", str().c_str());
}

SmeAckMonitor::~SmeAckMonitor()
{
	//__trace2__("monitor deleted: %s", str().c_str());
}

string SmeAckMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", startTime = " << startTime;
	return s.str();
}

GenericNackMonitor::GenericNackMonitor(uint32_t seqNum, PduData* pduData,
	PduFlag flag)
: PduMonitor(pduData->submitTime, pduData->submitTime, pduData, flag),
	sequenceNumber(seqNum)
{
	//__trace2__("monitor created: %s", str().c_str());
}

GenericNackMonitor::~GenericNackMonitor()
{
	//__trace2__("monitor deleted: %s", str().c_str());
}

string GenericNackMonitor::str() const
{
	ostringstream s;
	s << PduMonitor::str() << ", sequenceNumber = " << sequenceNumber;
	return s.str();
}

}
}
}

