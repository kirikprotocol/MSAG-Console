#ifndef TEST_UTIL_UTIL
#define TEST_UTIL_UTIL

#include <ostream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

/**
 * Флаг задающий выполнение случайной тестовой процедуры в рамках test case.
 */
const int RAND_TC = -1;

/**
 * Флаг задающий выполнение всех тестовых процедур в рамках test case.
 */
const int ALL_TC = 0;

int rand0(int maxValue);

int rand1(int maxValue);

std::auto_ptr<uint8_t> rand_uint8_t(int length);

std::auto_ptr<char> rand_char(int length);

/**
 * Класс для хранения результатов выполнения test case.
 */
class TCResult
{
private:
	const std::string id;
	const int choice;
	std::vector<int> failures;

public:
	TCResult(const std::string& id, const int choice);
	TCResult(TCResult& tcRes);
	const std::string& getId() const;
	int getChoice() const;
	void addFailure(int subTC);
	const std::vector<int>& getFailures() const;
	bool value();
	bool operator== (const TCResult& tcRes) const;
	friend std::ostream& operator<< (std::ostream& os, const TCResult& res);
};

/**
 * Позволяет выбрать нужную или все процедуры в test case в зависимости от 
 * параметров.
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
	int value();
	bool check();
	TCSelector& operator++ (int);
	int getChoice();
};

//TCResult inline member functions definitions
inline TCResult::TCResult(const std::string& _id, const int _choice)
	: id(_id), choice(_choice) {}

inline TCResult::TCResult(TCResult& tcRes)
	: id(tcRes.getId()), choice(tcRes.getChoice()), 
	failures(tcRes.getFailures()) {}

inline const std::string& TCResult::getId() const
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

inline bool TCResult::value()
{
	return failures.empty();
}

//TCSelector inline member functions definitions
inline int TCSelector::value()
{
	return i;
}

inline bool TCSelector::check()
{
	return (val == 0 ? i <= maxVal : first);
}
	
inline TCSelector& TCSelector::operator++ (int)
{
	i++;
	first = false;
	return *this;
}

inline int TCSelector::getChoice()
{
	return (val == -1 ? i : val);
}

}
}
}

#endif /* TEST_UTIL_UTIL */
