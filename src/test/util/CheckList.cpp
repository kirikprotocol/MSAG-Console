#include "CheckList.hpp"
#include <map>
#include <string>
#include <fstream>

namespace smsc {
namespace test {
namespace util {

using namespace std;

const string CheckList::UNIT_TEST = "unit_test";
const string CheckList::SYSTEM_TEST = "system_test";
const string CheckList::STANDARDS_TEST = "standards_test";
CheckList::CheckListMap* CheckList::clists = new CheckList::CheckListMap();

CheckList& CheckList::getCheckList(const string& name)
{
	CheckList* chk = (*clists)[name];
	if (chk == NULL)
	{
		chk = new CheckList(name);
		(*clists)[name] = chk;
	}
	return *chk;
}

CheckList::CheckList(const string& name_)
{
	name = name_;
	counter = 1;
	string tmp = "./" + name + ".chk";
	os.open(tmp.data());
}

void CheckList::startNewGroup(const string& groupName)
{
	counter = 1;
	os << endl << groupName << endl;
	os << "№|Тест|Id|Результат" << endl;
}

void CheckList::writeResult(const string& testCaseDesc, TCResult result)
{
	os << counter++ << "|" << testCaseDesc << "|";
	result.print(os);
	os << "|" << (result.value() ? "Ok" : "Failed") << endl;
}

}
}
}

