#ifndef TEST_UTIL_UTIL
#define TEST_UTIL_UTIL

#include <ostream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

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

void init_rand();

int rand2(int minValue, int maxValue);

int rand0(int maxValue);

int rand1(int maxValue);

std::auto_ptr<uint8_t> rand_uint8_t(int length);

std::auto_ptr<char> rand_char(int length);

/**
 * Класс для хранения результатов выполнения тест кейса.
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
