#ifndef TEST_UTIL_UTIL
#define TEST_UTIL_UTIL

#include <ostream>
#include <sstream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

using std::ostream;
using std::vector;

//����� ���������� ���� ������
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

//����� ����������� ���������� ��������� �����
const int RAND_LAT = 1;
const int RAND_RUS = 2;
const int RAND_NUM = 4;
const int RAND_SYM = 8;
const int RAND_DEF_SMS = 16;
const int RAND_LAT_NUM = RAND_LAT + RAND_NUM;

static const char* digitChars = "1234567890";
static const char* latinChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const char* rusChars = "�����Ũ����������������������������������������������������������";
static const char* symbolChars = "~ !@#$%^&*()-_=+\\|{[}];:'\",<.>/?";

inline const char* nvl(const char* str) { return (str ? str : ""); }

void init_rand();

int rand2(int minValue, int maxValue);

int rand0(int maxValue);

int rand1(int maxValue);

double randDouble(int digits, int precision);

std::auto_ptr<uint8_t> rand_uint8_t(int length);
void rand_uint8_t(int length, uint8_t* buf);

std::auto_ptr<char> rand_char(int length, int type = RAND_LAT_NUM);
void rand_char(int length, char* buf, int type = RAND_LAT_NUM);

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

ostream& operator<< (ostream& os, int8_t val);
//ostream& operator<< (ostream& os, int16_t val);
//ostream& operator<< (ostream& os, int32_t val);

ostream& operator<< (ostream& os, uint8_t val);
//ostream& operator<< (ostream& os, uint16_t val);
//ostream& operator<< (ostream& os, uint32_t val);

}
}
}

#endif /* TEST_UTIL_UTIL */
