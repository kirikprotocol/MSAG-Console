#ifndef TEST_UTIL_TC_RESULT_FILTER
#define TEST_UTIL_TC_RESULT_FILTER

#include "Util.hpp"
#include <vector>
#include <map>

namespace smsc {
namespace test {
namespace util {

class CheckList;

/**
 * Стек для результатов выполнения test case
 * 
 * @author bryz
 */
class TCResultStack : public std::vector<TCResult*>
{
public:
	~TCResultStack();
	bool operator== (const TCResult& result) const;
	bool operator== (const TCResultStack& stack) const;
};

/**
 * Фильтр для результатов выполнения test cases
 * 
 * @author bryz
 */
class TCResultFilter
{
public:
	//friend void CheckList::writeResult(const TCResultFilter&);
	friend class CheckList;

	struct TCValue
	{
		TCValue(const std::string& _description)
			: description(_description) {}
		std::string description;
		std::vector<TCResultStack*> tcStacks;
	};
	typedef std::map<std::string, TCValue*> TCMap;

	~TCResultFilter();

	/**
	 * Устанавливает соответствие между test case id и текстовым описанием test
	 * case.
	 */
	void registerTC(const std::string& tcId, const std::string& tcDescription);

	/**
	 * Добавить результаты тестирования test case
	 */
	void addResult(TCResult& result);

	/**
	 * Добавить результаты тестирования test case
	 */
	void addResultStack(TCResultStack& stack);

private:
	TCMap resmap;

	TCValue& getTCValue(const std::string& tcId);
};

}
}
}

#endif /* TEST_UTIL_TC_RESULT_FILTER */
