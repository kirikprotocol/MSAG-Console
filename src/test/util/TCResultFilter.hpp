#ifndef TEST_UTIL_TC_RESULT_FILTER
#define TEST_UTIL_TC_RESULT_FILTER

#include "Util.hpp"
#include <string>
#include <vector>
#include <map>

namespace smsc {
namespace test {
namespace util {

/**
 * Стек для результатов выполнения test case
 * 
 * @author bryz
 */
class TCResultStack : public std::vector<const TCResult*>
{
public:
	virtual ~TCResultStack();
	void push_back(const TCResult* result);
	bool operator== (const TCResult& result) const;
	bool operator== (const TCResultStack& stack) const;
};

class TCResultStackList : public std::vector<const TCResultStack*>
{
public:
	virtual ~TCResultStackList();
	friend std::ostream& operator<< (std::ostream& os,
		const TCResultStackList& stackList);
};

/**
 * Фильтр для результатов выполнения test cases
 * 
 * @author bryz
 */
class TCResultFilter
{
public:
	~TCResultFilter();

	/**
	 * Добавить результаты тестирования test case
	 */
	void addResult(const TCResult* result);

	/**
	 * Добавить результаты тестирования test case
	 */
	void addResultStack(const TCResultStack& stack);

	/**
	 * @return отфильтрованные стеки результатов
	 */
	const TCResultStackList* getResults(const char* tcId);

private:
	typedef std::map<const std::string, TCResultStackList*> TCMap;
	TCMap resmap;
};

}
}
}

#endif /* TEST_UTIL_TC_RESULT_FILTER */
