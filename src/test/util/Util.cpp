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

TCSelector::TCSelector(int _val, int _maxVal, int _base) : pos(0)
{
	switch (_val)
	{
		case ALL_TC:
			val = new int[_maxVal];
			for (int i = 0; i < _maxVal; i++)
			{
				val[i] = _base + i + 1;
			}
			size = _maxVal;
			choice = ALL_TC;
			break;
		case RAND_TC:
			val = new int(rand1(_maxVal));
			size = 1;
			choice = *val;
			break;
		case RAND_SET_TC:
			size = rand0(_maxVal);
			if (size > 0)
			{
				val = new int[size];
				for (int i = 0; i < size; i++)
				{
					val[i] = _base + rand1(_maxVal);
				}
			}
			choice = RAND_SET_TC;
			break;
		default:
			val = new int(_val);
			size = 1;
			choice = _val;
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
