#include "PduUtil.hpp"
#include "test/TestConfig.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using namespace smsc::test;

DeliveryIterator& DeliveryIterator::operator++()
{
	attempt++;
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
	return *this;
}

time_t PduReceiptFlag::eval(time_t time, int& attempt, time_t& diff) const
{
	__require__(time);
	DeliveryIterator it(startTime, endTime);
	diff = INT_MAX;
	time_t tmp = time - it.getTime();
	while (it.getTime() && abs(tmp) < abs(diff))
	{
		diff = tmp;
		attempt = it.getAttempt();
		tmp = time - (it++).getTime();
	}
	return it.getTime();
}

time_t PduReceiptFlag::getNextTime(time_t t) const
{
	int attempt;
	time_t diff;
	return eval(t, attempt, diff);
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
		time_t nextTime = eval(recvTime, attempt, diff);
		if (lastTime)
		{
			__require__(lastTime >= startTime && lastTime <= endTime + timeCheckAccuracy);
			eval(lastTime, lastAttempt, lastDiff);
		}
		__trace2__("PduReceiptFlag::checkSchedule(): this = %p, startTime = %ld, endTime = %ld, recvTime = %ld, attempt = %d, diff = %ld, lastTime = %ld, lastAttempt = %d, lastDiff = %ld",
			this, startTime, endTime, recvTime, attempt, diff, lastTime, lastAttempt, lastDiff);
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
	__trace2__("PduReceiptFlag::update(): this = %p, recvTime = %ld, accepted = %s",
		this, recvTime, accepted ? "true" : "false");
	vector<int> res;
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
	eval(checkTime, attempt, diff);
	if (lastTime)
	{
		__require__(lastTime >= startTime && lastTime <= endTime + timeCheckAccuracy);
		eval(lastTime, lastAttempt, lastDiff);
	}
	if (attempt - lastAttempt != 1)
	{
		__trace2__("PduReceiptFlag::isPduMissing(): this = %p, startTime = %ld, endTime = %ld, checkTime = %ld, attempt = %d, diff = %ld, lastTime = %ld, lastAttempt = %d, lastDiff = %ld",
			this, startTime, endTime, checkTime, attempt, diff, lastTime, lastAttempt, lastDiff);
		return true;
	}
	return false;
}

Mutex PduData::mutex = Mutex();
uint32_t PduData::counter = 1;

}
}
}

