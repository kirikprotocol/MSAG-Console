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
		time_t diff, lastDiff;
		time_t nextTime = eval(recvTime, attempt, diff);
		__trace2__("PduReceiptFlag::update(): this = 0x%x, recvTime = %ld, startTime = %ld, endTime = %ld, attempt = %d, diff = %d, nextTime = %ld",
			this, recvTime, startTime, endTime, attempt, diff, nextTime);
		if (lastTime)
		{
			__require__(lastTime >= startTime && lastTime <= endTime + timeCheckAccuracy);
			eval(lastTime, lastAttempt, lastDiff);
		}
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

bool PduReceiptFlag::isPduMissing(time_t checkTime)
{
	if (checkTime < startTime || flag != PDU_REQUIRED_FLAG)
	{
		return false;
	}
	else if (checkTime > endTime + timeCheckAccuracy)
	{
		return true;
	}
	int attempt, lastAttempt = -1;
	time_t diff;
	eval(checkTime, attempt, diff);
	__trace2__("PduReceiptFlag::isPduMissing(): this = 0x%x, recvTime = %ld, statTime = %ld, endTime = %ld, attempt = %d, diff = %d",
		this, checkTime, startTime, endTime, attempt, diff);
	if (lastTime)
	{
		__require__(lastTime >= startTime && lastTime <= endTime + timeCheckAccuracy);
		eval(lastTime, lastAttempt, diff);
	}
	return (attempt - lastAttempt != 1);
}

Mutex PduData::mutex = Mutex();
uint32_t PduData::counter = 1;

}
}
}

