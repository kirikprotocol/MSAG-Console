#ifndef TEST_UTIL_CHECK_LIST
#define TEST_UTIL_CHECK_LIST

#include "Util.hpp"
#include "TCResultFilter.hpp"
#include <fstream>
#include <map>
#include <string>

namespace smsc {
namespace test {
namespace util {

/**
 * Вспомогательный класс для записи результатов в checklist.
 */
class CheckList
{
public:
	static const std::string UNIT_TEST;
	static const std::string SYSTEM_TEST;
	static const std::string STANDARDS_TEST;

	typedef std::map<std::string, CheckList*> CheckListMap;
	
	/**
	 * @return именованый checklist
	 */
	static CheckList& getCheckList(const std::string& name);

	CheckList(const std::string& name);
	~CheckList();

	/**
	 * Начать новую группу/таблицу в checklist.
	 */
	void startNewGroup(const std::string& groupName);

	/**
	 * Записать результаты тестирования (test case) в текущцю группу/таблицу
	 */
	void writeResult(const std::string& testCaseDesc, TCResult& result);

	/**
	 * Записать результаты тестирования (группы test case) в текущцю 
	 * группу/таблицу
	 */
	void writeResult(TCResultFilter& resultFilter);

private:
	static CheckListMap* clists;
	std::string name;
	std::ofstream os;
	int counter;
};

}
}
}

#endif /* TEST_UTIL_CHECK_LIST */
