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
	if (minValue > maxValue)
	{
		int tmp = (int) ((minValue - maxValue + 1) * (float) rand() / (RAND_MAX + 1.0));
		return maxValue + tmp;
	}
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

static string randChars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

auto_ptr<uint8_t> rand_uint8_t(int length)
{
	uint8_t* res = new uint8_t[length];
	for (int i = 0; i < length; i++)
	{
		res[i] = randChars[rand0(randChars.size() - 1)];
	}
	return auto_ptr<uint8_t>(res);
}

void rand_uint8_t(int length, uint8_t* buf)
{
	for (int i = 0; i < length; i++)
	{
		buf[i] = randChars[rand0(randChars.size() - 1)];
	}
}

auto_ptr<char> rand_char(int length)
{
	char* res = new char[length + 1];
	for (int i = 0; i < length; i++)
	{
		res[i] = randChars[rand0(randChars.size() - 1)];
	}
	res[length] = 0;
	return auto_ptr<char>(res);
}

void rand_char(int length, char* buf)
{
	for (int i = 0; i < length; i++)
	{
		buf[i] = randChars[rand0(randChars.size() - 1)];
	}
	buf[length] = 0;
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

TCSelector::~TCSelector()
{
	if (val)
	{
		delete[] val;
	}
}

bool TCSelector::check() const
{
	return (pos < size);
}
	
TCSelector& TCSelector::operator++ (int)
{
	pos++;
	return *this;
}

int TCSelector::getChoice() const
{
	return choice;
}

int TCSelector::value() const
{
	return val[pos];
}

int TCSelector::value1(int num1) const
{
	return 1 + (value() - 1) % num1;
}

int TCSelector::value2(int num1) const
{
	return 1 + (value() - 1) / num1;
}

int TCSelector::value1(int num1, int num2) const
{
	return 1 + ((value() - 1) % (num1 * num2)) % num1;
}

int TCSelector::value2(int num1, int num2) const
{
	return 1 + ((value() - 1) % (num1 * num2)) / num1;
}

int TCSelector::value3(int num1, int num2) const
{
	return 1 + (value() - 1) / (num1 * num2);
}

}
}
}
