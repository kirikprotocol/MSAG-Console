#include "PduUtil.hpp"
#include "test/TestConfig.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using namespace smsc::test;

DeliveryIterator::DeliveryIterator(time_t start, time_t end)
	: startTime(start), endTime(end), attempt(0), time(start)
{
	__require__(start <= end);
}

DeliveryIterator& DeliveryIterator::operator++()
{
	static const int rescheduleSize =
		sizeof(rescheduleTimes) / sizeof(*rescheduleTimes);
	if (attempt < rescheduleSize)
	{
		time += rescheduleTimes[attempt];
	}
	else
	{
		time += rescheduleTimes[rescheduleSize - 1];
	}
	if (time > endTime)
	{
		time = 0;
	}
	attempt++;
	return *this;
}

void PduReceiptFlag::eval(time_t time, int& attempt, time_t& diff,
	time_t& nextTime, time_t& calcTime) const
{
	__require__(time);
	DeliveryIterator it(startTime, endTime);
	diff = INT_MAX;
	for (; it.getTime(); it++)
	{
		__trace2__("it.getTime() = %ld", it.getTime());
		time_t curDiff = it.getTime() - time;
		if (abs(curDiff) > abs(diff))
		{
			break;
		}
		diff = curDiff;
		attempt = it.getAttempt();
		calcTime = it.getTime();
	}
	nextTime = it.getTime();
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
		res.push_back(1);
	}
	else if (recvTime > endTime + timeCheckAccuracy)
	{
		res.push_back(2);
	}
	//else if (lastTime && (lastTime < startTime || lastTime > endTime))
	else
	{
		int attempt, lastAttempt = -1;
		time_t diff, lastDiff = 0;
		time_t nextTime, calcTime, lastNextTime, lastCalcTime;
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

