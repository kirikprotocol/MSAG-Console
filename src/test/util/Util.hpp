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

int rand1(int maxValue);

std::auto_ptr<uint8_t> rand_uint8_t(int length);

std::auto_ptr<char> rand_char(int length);

/**
 * ����� ��� �������� ����������� ���������� test case.
 */
class TCResult
{
private:
	const char* id;
	const int choice;
	std::vector<int> failures;

public:
	TCResult(const char* id, const int choice = 1);
	TCResult(const TCResult& tcRes);
	const char* getId() const;
	int getChoice() const;
	void addFailure(int subTC);
	const std::vector<int>& getFailures() const;
	bool value() const;
	bool operator== (const TCResult& tcRes) const;
	friend std::ostream& operator<< (std::ostream& os, const TCResult& res);
};

/**
 * ��������� ������� ������ ��� ��� ��������� � test case � ����������� �� 
 * ����������.
 */
class TCSelector
{
private:
	int i;
	bool first;
	const int val;
	const int maxVal;

public:
	TCSelector(int val, int maxVal);
	int value() const;
	bool check() const;
	TCSelector& operator++ (int);
	int getChoice() const;
};

//TCResult inline member functions definitions
inline TCResult::TCResult(const char* _id, const int _choice)
	: id(_id), choice(_choice) {}

inline TCResult::TCResult(const TCResult& tcRes)
	: id(tcRes.getId()), choice(tcRes.getChoice()), 
	failures(tcRes.getFailures()) {}

inline const char* TCResult::getId() const
{
	return id;
}

inline int TCResult::getChoice() const
{
	return choice;
}

inline void TCResult::addFailure(int subTC)
{
	failures.push_back(subTC);
}

inline const std::vector<int>& TCResult::getFailures() const
{
	return failures;
}

inline bool TCResult::value() const
{
	return failures.empty();
}

//TCSelector inline member functions definitions
inline int TCSelector::value() const
{
	return i;
}

inline bool TCSelector::check() const
{
	return (val == 0 ? i <= maxVal : first);
}
	
inline TCSelector& TCSelector::operator++ (int)
{
	i++;
	first = false;
	return *this;
}

inline int TCSelector::getChoice() const
{
	return (val == -1 ? i : val);
}

}
}
}

#endif /* TEST_UTIL_UTIL */
