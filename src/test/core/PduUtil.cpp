#include "PduUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

void PduReceiptFlag::eval(time_t time, int& attempt, time_t& diff,
	time_t& nextTime, time_t& calcTime) const
{
	__require__(time);
	attempt = lastAttempt;
	diff = INT_MAX;
	nextTime = lastTime ? lastTime : startTime;
	for (;; attempt++)
	{
		//__trace2__("eval(): nextTime = %ld", nextTime);
		time_t curDiff = nextTime - time;
		if (abs(curDiff) > abs(diff))
		{
			break;
		}
		diff = curDiff;
		calcTime = nextTime;
		if (attempt < rescheduleSize)
		{
			nextTime += rescheduleTimes[attempt];
		}
		else
		{
			nextTime += rescheduleTimes[rescheduleSize - 1];
		}
		if (nextTime > endTime)
		{
			nextTime = 0;
		}
	}
	attempt--;
}

time_t PduReceiptFlag::getNextTime(time_t t) const
{
	int attempt;
	time_t diff, nextTime, calcTime;
	eval(t, attempt, diff, nextTime, calcTime);
	return nextTime;
}

vector<int> PduReceiptFlag::checkSchedule(time_t recvTime) const
{
	vector<int> res;
	if (recvTime < startTime)
	{
		__trace2__("PduReceiptFlag::checkSchedule(): this = %p, startTime = %ld, endTime = %ld, recvTime = %ld is less startTime",
			this, startTime, endTime, recvTime);
		res.push_back(1);
	}
	else if (recvTime > endTime + timeCheckAccuracy)
	{
		__trace2__("PduReceiptFlag::checkSchedule(): this = %p, startTime = %ld, endTime = %ld, recvTime = %ld is greater endTime",
			this, startTime, endTime, recvTime);
		res.push_back(2);
	}
	//else if (lastTime && (lastTime < startTime || lastTime > endTime))
	else
	{
		int attempt, lastAttempt = -1;
		time_t diff, lastDiff = 0;
		time_t nextTime, calcTime, lastNextTime = 0, lastCalcTime = 0;
		eval(recvTime, attempt, diff, nextTime, calcTime);
		if (lastTime)
		{
			__require__(lastTime >= startTime && lastTime <= endTime + timeCheckAccuracy);
			eval(lastTime, lastAttempt, lastDiff, lastNextTime, lastCalcTime);
		}
		__trace2__("PduReceiptFlag::checkSchedule(): this = %p, startTime = %ld, endTime = %ld, recvTime = %ld, attempt = %d, calcTime = %ld, diff = %ld, lastTime = %ld, lastAttempt = %d, lastCalcTime = %ld, lastDiff = %ld",
			this, startTime, endTime, recvTime, attempt, calcTime, diff, lastTime, lastAttempt, lastCalcTime, lastDiff);
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

vector<int> PduReceiptFlag::update(time_t recvTime, bool accepted)
{
	vector<int> res;
	int prevFlag = flag;
	switch (flag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			{
				time_t nextTime = getNextTime(recvTime);
				if (accepted || !nextTime)
				{
					flag = PDU_RECEIVED_FLAG;
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
	__trace2__("PduReceiptFlag::update(): this = %p, startTime = %ld, endTime = %ld, recvTime = %ld, accepted = %s, flag = %d, prevFlag = %d",
		this, startTime, endTime, recvTime, accepted ? "true" : "false", flag, prevFlag);
	lastTime = recvTime;
	return res;
}

bool PduReceiptFlag::isPduMissing(time_t checkTime) const
{
	__trace2__("PduReceiptFlag::isPduMissing(): this = %p, statTime = %ld, endTime = %ld, checkTime = %ld",
		this, startTime, endTime, checkTime);
	if (checkTime < startTime || flag != PDU_REQUIRED_FLAG)
	{
		return false;
	}
	else if (checkTime > endTime + timeCheckAccuracy)
	{
		return true;
	}
	int attempt, lastAttempt = -1;
	time_t diff, lastDiff = 0;
	time_t nextTime, calcTime, lastNextTime, lastCalcTime;
	eval(checkTime, attempt, diff, nextTime, calcTime);
	if (lastTime)
	{
		__require__(lastTime >= startTime && lastTime <= endTime + timeCheckAccuracy);
		eval(lastTime, lastAttempt, lastDiff, lastNextTime, lastCalcTime);
	}
	if (attempt - lastAttempt != 1)
	{
		__trace2__("PduReceiptFlag::isPduMissing(): this = %p, startTime = %ld, endTime = %ld, checkTime = %ld, attempt = %d, calcTime = %ld, diff = %ld, lastTime = %ld, lastAttempt = %d, lastCalcTime = %ld, lastDiff = %ld",
			this, startTime, endTime, checkTime, attempt, calcTime, diff, lastTime, lastAttempt, lastCalcTime, lastDiff);
		return true;
	}
	return false;
}

Mutex PduData::mutex = Mutex();
uint32_t PduData::counter = 1;

}
}
}

