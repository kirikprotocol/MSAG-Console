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

time_t PduReceiptFlag::eval(time_t time, int& attempt, time_t& diff)
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

vector<int> PduReceiptFlag::update(time_t recvTime, bool accepted, time_t& nextTime)
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
		nextTime = eval(recvTime, attempt, diff);
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
		lastTime = recvTime;
		switch (flag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				if (accepted || !nextTime)
				{
					flag = PDU_RECEIVED_FLAG;
				}
				break;
			case PDU_RECEIVED_FLAG:
				res.push_back(6);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				res.push_back(7);
				break;
			default:
				__unreachable__("Unknown flag");
		}
	}
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

}
}
}

