#include "Util.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"
#include <cstdlib>
#include <ctime>

namespace smsc {
namespace test {
namespace util {

using namespace std;
using namespace smsc::core::synchronization; //Mutex, MutexGuard

int hash(const char* s)
{
	int res = 0;
	for (; *s; s++)
	{
		res += res * 31 + *s;
	}
	return res;
}

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

double randDouble(int digits, int precision)
{
	__require__(digits > 0 && precision >= 0 && digits >= precision);
	char tmp[digits + 3]; //sign + point + null terminator
	tmp[0] = rand0(1) ? '+' : '-';
	for (int i = 1; i <= digits + 1; i++)
	{
		tmp[i] = digitChars[rand0(9)];
	}
	tmp[digits - precision + 1] = '.';
	tmp[digits + 2] = 0;
	double val;
	sscanf(tmp, "%lf", &val);
	return val;
}

auto_ptr<uint8_t> rand_uint8_t(int length)
{
	uint8_t* res = new uint8_t[length];
	for (int i = 0; i < length; i++)
	{
		res[i] = rand0(255);
	}
	return auto_ptr<uint8_t>(res);
}

void rand_uint8_t(int length, uint8_t* buf)
{
	for (int i = 0; i < length; i++)
	{
		buf[i] = rand0(255);
	}
}

auto_ptr<char> rand_char(int length, int type)
{
	char* buf = new char[length + 1];
	rand_char(length, buf, type);
	return auto_ptr<char>(buf);
}

void rand_char(int length, char* buf, int type)
{
	vector<int> charTypes;
	if (type & RAND_LAT) { charTypes.push_back(RAND_LAT); }
	if (type & RAND_RUS) { charTypes.push_back(RAND_RUS); }
	if (type & RAND_NUM) { charTypes.push_back(RAND_NUM); }
	if (type & RAND_SYM) { charTypes.push_back(RAND_SYM); }
	__require__(charTypes.size());
	for (int i = 0; i < length; i++)
	{
		if ((type & RAND_WS) && !rand0(15))
		{
			buf[i] = wsChars[rand0(wsCharsLen - 1)];
			continue;
		}
		switch (charTypes[rand0(charTypes.size() - 1)])
		{
			case RAND_LAT:
				buf[i] = latinChars[rand0(latinCharsLen - 1)];
				break;
			case RAND_RUS:
				buf[i] = rusChars[rand0(rusCharsLen - 1)];
				break;
			case RAND_NUM:
				buf[i] = digitChars[rand0(digitCharsLen - 1)];
				break;
			case RAND_SYM:
				buf[i] = symbolChars[rand0(symbolCharsLen - 1)];
				break;
			default:
				__unreachable__("Invalid char type");
		}
	}
	buf[length] = 0;
}

const string TestCaseId::str() const
{
	ostringstream s;
	s << "{";
	copy(id.begin(), id.end(), ostream_iterator<int>(s, ", "));
	s << "}";
	return s.str();
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

ostream& operator<< (ostream& os, int8_t val)
{
	os << (int) val;
	return os;
}

ostream& operator<< (ostream& os, uint8_t val)
{
	os << (unsigned int) val;
	return os;
}

}
}
}
