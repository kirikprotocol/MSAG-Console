#include "CheckList.hpp"
#include "util/debug.h"
#include <map>
#include <fstream>

namespace smsc {
namespace test {
namespace util {

using namespace std;

const char* CheckList::UNIT_TEST = "unit_test";
const char* CheckList::SYSTEM_TEST = "system_test";
const char* CheckList::STANDARDS_TEST = "standards_test";
CheckList::CheckListMap* CheckList::clists = new CheckList::CheckListMap();

CheckList& CheckList::getCheckList(const char* name)
{
	CheckList* chk = (*clists)[name];
	if (!chk)
	{
		chk = new CheckList(name);
		(*clists)[name] = chk;
	}
	return *chk;
}

CheckList::CheckList(const char* name_)
	: name(name_), counter(1)
{
	char fileName[100];
	sprintf(fileName, "./%s.chk", name);
	os.open(fileName);
}

void CheckList::startNewGroup(const char* groupName, const char* packageName)
{
	counter = 1;
	os << endl << "|" << groupName << endl;
	os << "|Покрытие кода для " << packageName << "|" << endl;
	os << "№|Тест|Результат|Стек" << endl;
}

void CheckList::writeResult(const char* tcDesc, const TCResult* result)
{
	__require__(tcDesc);
	os << counter++ << "|" << tcDesc << "|";
	if (result)
	{
		os << (result->value() ? "Да" : "Нет") << *result << endl;
	}
	else
	{
		os << "-|" << endl;
	}
	os.flush();
}

void CheckList::writeResult(const char* tcDesc, const TCResultStackList* stackList)
{
	__require__(tcDesc);
	os << counter++ << "|" << tcDesc << "|";
	if (stackList)
	{
		os << (stackList->size() == 0 ? "Да" : "Нет") << "|"
			<< *stackList << endl;
	}
	else
	{
		os << "-|" << endl;
	}
	os.flush();
}

}
}
}

