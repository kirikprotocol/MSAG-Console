#ifndef TEST_UTIL_UTIL
#define TEST_UTIL_UTIL

#include <ostream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

/**
 * ���� �������� ���������� ��������� �������� ��������� � ������ test case.
 */
const int RAND_TC = -1;

/**
 * ���� �������� ���������� ���� �������� �������� � ������ test case.
 */
const int ALL_TC = 0;

int rand0(int maxValue);

std::auto_ptr<uint8_t> rand_uint8_t(int length);

/**
 * ��������� ������� ������ ��� ��� ��������� � test case � ����������� �� 
 * ����������.
 */
class TCSelector
{
private:
	int i;
	int val;
	int maxVal;

public:
	TCSelector(int _val, int _maxVal);
	int value();
	bool check();
	TCSelector& operator++ (int);
};

class TCResult
{
private:
	const char* id;
	std::vector<int> failedTC;

public:
	TCResult(const char* id);
	void addFailure(int subTC);
	bool value();
	void print(std::ostream& os);
};

}
}
}

#endif /* TEST_UTIL_UTIL */
