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
	return (int) ((maxValue + 1) * rand() / (RAND_MAX + 1.0));
}

int rand1(int maxValue)
{
	return 1 + (int) (maxValue * rand() / (RAND_MAX + 1.0));
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

auto_ptr<char> rand_char(int length)
{
	char* res = new char[length];
	for (int i = 0; i < length; i++)
	{
		res[i] = 65 + rand0(26);
	}
	return auto_ptr<char>(res);
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

TCSelector::TCSelector(int _val, int _maxVal)
	: val(_val), maxVal(_maxVal), first(true)
{
	if (val < 0)
	{
		i = rand1(maxVal);
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

ostream& operator<< (ostream& os, const TCResult& res)
{
	os << res.getId() << "(" << res.getChoice() << ")";
	os << "{";
	const vector<int>& failures = res.getFailures();
	for (int i = 0; i < failures.size(); i++)
	{
		if (i > 0)
		{
			os << ",";
		}
		os << failures[i];
	}
	os << "}";
}

}
}
}
