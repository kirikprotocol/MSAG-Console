#ifndef TEST_UTIL_UTIL
#define TEST_UTIL_UTIL

#include <ostream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

using std::vector;

/**
 * ���� �������� ���������� ���� �������� �������� � ������ ���� �����.
 */
const int ALL_TC = 0;

/**
 * ���� �������� ���������� ��������� �������� ��������� � ������ ���� �����.
 */
const int RAND_TC = -1;

/**
 * ���� �������� ���������� ���� �������� ��������, ��������� �������� ���������,
 * ���������������� ������ ��������� �������� �������� ��� ������ ������ � 
 * ������ ���� �����.
 */
const int RAND_SET_TC = -2;

void init_rand();

int rand2(int minValue, int maxValue);

int rand0(int maxValue);

int rand1(int maxValue);

std::auto_ptr<uint8_t> rand_uint8_t(int length);

std::auto_ptr<char> rand_char(int length);
void rand_char(int length, char* buf);

template <class T>
struct Deletor
{
	T* operator()(T* obj)
	{
		if (obj) { delete obj; }
		return NULL;
	}
};

/**
 * ��������� ������� ������ ��� ��� ��������� � ���� ����� � ����������� �� 
 * ����������.
 */
class TCSelector
{
private:
	int* val;
	int pos; //������� ������� �� ������� val
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

//TCSelector inline member functions definitions
inline TCSelector::~TCSelector()
{
	if (val)
	{
		delete[] val;
	}
}

inline bool TCSelector::check() const
{
	return (pos < size);
}
	
inline TCSelector& TCSelector::operator++ (int)
{
	pos++;
	return *this;
}

inline int TCSelector::getChoice() const
{
	return choice;
}

inline int TCSelector::value() const
{
	return val[pos];
}

inline int TCSelector::value1(int num1) const
{
	return 1 + (val[pos] - 1) % num1;
}

inline int TCSelector::value2(int num1) const
{
	return 1 + (val[pos] - 1) / num1;
}

inline int TCSelector::value1(int num1, int num2) const
{
	return 1 + ((val[pos] - 1) % (num1 * num2)) % num1;
}

inline int TCSelector::value2(int num1, int num2) const
{
	return 1 + ((val[pos] - 1) % (num1 * num2)) / num1;
}

inline int TCSelector::value3(int num1, int num2) const
{
	return 1 + (val[pos] - 1) / (num1 * num2);
}

}
}
}

#endif /* TEST_UTIL_UTIL */
