#ifndef TEST_UTIL_UTIL
#define TEST_UTIL_UTIL

#include <ostream>
#include <sstream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

using std::vector;

//флаги выполнения тест кейсов
/**
 * Флаг задающий выполнение всех тестовых процедур в рамках тест кейса.
 */
const int ALL_TC = 0;

/**
 * Флаг задающий выполнение случайной тестовой процедуры в рамках тест кейса.
 */
const int RAND_TC = -1;

/**
 * Флаг задающий выполнение всех тестовых процедур, случайной тестовой процедуры,
 * неупорядоченного набора случайных тестовых процедур или вообще ничего в 
 * рамках тест кейса.
 */
const int RAND_SET_TC = -2;

//флаги управляющие генерацией случайных строк

const int RAND_ALPHA_NUM = 0;
const int RAND_ALPHA = 1;
const int RAND_NUM = 2;

void init_rand();

int rand2(int minValue, int maxValue);

int rand0(int maxValue);

int rand1(int maxValue);

std::auto_ptr<uint8_t> rand_uint8_t(int length);
void rand_uint8_t(int length, uint8_t* buf);

std::auto_ptr<char> rand_char(int length, int type = RAND_ALPHA_NUM);
void rand_char(int length, char* buf, int type = RAND_ALPHA_NUM);

template <class T>
struct Deletor
{
	T* operator()(T* obj)
	{
		if (obj) { delete obj; }
		return NULL;
	}
};

template <class T>
class Mask
{
	T mask;
public:
	Mask(T val) : mask(val) {}
	bool operator[](int index) const
	{
		__require__(index < sizeof(T) * 8);
		return ((mask >> index) & 0x1);
	}
	const char* str() const
	{
		std::ostringstream s;
		s << std::hex << mask;
		return s.str().c_str();
	}
};

/**
 * Позволяет выбрать нужную или все процедуры в тест кейсе в зависимости от 
 * параметров.
 */
class TCSelector
{
private:
	int* val;
	int pos; //текущая позиция по массиву val
	int size;
	int choice;

public:
	TCSelector(int val, int maxVal, int base = 0);
	~TCSelector();
	bool check() const;
	TCSelector& operator++ (int);
	int getChoice() const;
	int value() const;
	int value1(int num1) const;
	int value2(int num1) const;
	int value1(int num1, int num2) const;
	int value2(int num1, int num2) const;
	int value3(int num1, int num2) const;
};

}
}
}

#endif /* TEST_UTIL_UTIL */
