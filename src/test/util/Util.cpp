#include "Util.hpp"
#include <cstdlib>
#include <ctime>

namespace smsc {
namespace test {
namespace util {

using namespace std;

//srand((unsigned) time(NULL));

int rand0(int maxValue)
{
	return (int) (maxValue * rand() / (RAND_MAX + 1.0));
}

auto_ptr<uint8_t> rand_uint8_t(int length)
{
	uint8_t* res = new uint8_t[length];
	for (int i = 0; i < length; i++)
	{
		res[i] = 65 + rand0(26);
	}
	return auto_ptr<uint8_t>(res);
}

TCSelector::TCSelector(int _val, int _maxVal)
{
	i = 1;
	val = _val;
	maxVal = _maxVal;
}

int TCSelector::value()
{
	if (val < 0)
	{
		return 1 + rand0(maxVal);
	}
	if (val == 0)
	{
		return i;
	}
	return val;
}

bool TCSelector::check()
{
	return (val == 0 ? i <= maxVal : i == 1);
}
	
TCSelector& TCSelector::operator++ (int)
{
	i++;
	return *this;
}

TCResult::TCResult(const char* _id)
{
	id = _id;
}

void TCResult::addFailure(int subTC)
{
	failedTC.push_back(subTC);
}

bool TCResult::value()
{
	return failedTC.empty();
}

void TCResult::print(ostream& os)
{
	os << id << "(";
	for (int i = 0; i < failedTC.size(); i++)
	{
		os << failedTC[i] << ",";
	}
	os << ")";
}

/*
int selectTestProc(int value, int maxValue)
{
	if (value < 0)
	{
		int num = (int) (maxValue * rand() / (RAND_MAX + 1.0));
		return num;
	}
	if (value > maxValue)
	{
		return value % maxValue;
	}
	return value;
}
*/

}
}
}
