#include "Util.hpp"
#include "core/synchronization/Mutex.hpp"
#include <cstdlib>
#include <ctime>

namespace smsc {
namespace test {
namespace util {

using namespace std;
using namespace smsc::core::synchronization; //Mutex, MutexGuard

void init_rand()
{
	static bool inited = false;
	static Mutex lock;
    MutexGuard mguard(lock);
	if (!inited)
	{
		inited = true;
		srand(time(NULL));
	}
}

int rand2(int minValue, int maxValue)
{
	int tmp = (int) ((maxValue - minValue + 1) * (float) rand() / (RAND_MAX + 1.0));
	return minValue + tmp;
}

int rand0(int maxValue)
{
	return rand2(0, maxValue);
}

int rand1(int maxValue)
{
	return rand2(1, maxValue);
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
	char* res = new char[length + 1];
	for (int i = 0; i < length; i++)
	{
		res[i] = 'A' + rand0(25);
	}
	res[length] = 0;
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

TCSelector::TCSelector(int _val, int _maxVal, int _base)
	: pos(0), val(NULL)
{
	switch (_val)
	{
		case ALL_TC:
			size = _maxVal;
			val = new int[size];
			for (int i = 0; i < size; i++)
			{
				val[i] = _base + i + 1;
			}
			choice = ALL_TC;
			break;
		case RAND_TC:
			size = 1;
			val = new int(_base + rand1(_maxVal));
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
			size = 1;
			val = new int(_val);
			choice = *val;
	}
}

ostream& operator<< (ostream& os, const TCResult& res)
{
	os << res.getId() << "(" << res.getChoice() << ")";
	const vector<int>& failures = res.getFailures();
	if (failures.size())
	{
		os << "{";
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
	return os;
}

}
}
}
