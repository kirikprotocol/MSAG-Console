#ifndef TEST_UTIL_CHECK_LIST
#define TEST_UTIL_CHECK_LIST

#include "Util.hpp"
#include "TCResultFilter.hpp"
#include <fstream>
#include <map>

namespace smsc {
namespace test {
namespace util {

/**
 * Вспомогательный класс для записи результатов в checklist.
 */
class CheckList
{
public:
	static const char* UNIT_TEST;
	static const char* SYSTEM_TEST;
	static const char* STANDARDS_TEST;

	typedef std::map<const char*, CheckList*> CheckListMap;
	
	/**
	 * @return именованый checklist
	 */
	static CheckList& getCheckList(const char* name);

	CheckList(const char* name);
	~CheckList();

	/**
	 * Начать новую группу/таблицу в checklist.
	 */
	void startNewGroup(const char* groupName, const char* packageName);

	/**
	 * Записать результаты тестирования тест кейсов в текущцю группу/таблицу
	 */
	void writeResult(const char* tcDesc, const TCResult* result);

	/**
	 * Записать результаты тестирования тест кейсов в текущую группу/таблицу
	 */
	void writeResult(const char* tcDesc, const TCResultStackList* stackList);

private:
	static CheckListMap* clists;
	const char* name;
	std::ofstream os;
	int counter;
};

}
}
}

#endif /* TEST_UTIL_CHECK_LIST */
