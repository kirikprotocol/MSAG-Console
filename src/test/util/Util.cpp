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

bool TCResult::operator== (const TCResult& tcRes) const
{
	bool res = id == tcRes.getId() && choice == tcRes.getChoice() &&
		failures.size() == tcRes.getFailures().size();
	if (res)
	{
		for (int i = 0; i < failures.size(); i++)
		{
			res &= failures[i] == tcRes.getFailures()[i];
		}
	}
	return res;
}

void TCResult::print(ostream& os)
{
	os << id << "(";
	for (int i = 0; i < failures.size(); i++)
	{
		if (i > 0)
		{
			os << ",";
		}
		os << failures[i];
	}
	os << ")";
}

TCSelector::TCSelector(int _val, int _maxVal)
	: val(_val), maxVal(_maxVal)
{
	if (val < 0)
	{
		i = 1 + rand0(maxVal);
	}
	else if (val == 0)
	{
		i = 1;
	}
	else
	{
		i = val;
	}
}

}
}
}
