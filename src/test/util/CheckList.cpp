#include <map>
#include <string>
#include <fstream>
#include "CheckList.h"

using namespace std;

namespace smsc {
namespace test {
namespace util {

const std::string CheckList::UNIT_TEST = "unit_test";
const std::string CheckList::SYSTEM_TEST = "system_test";
const std::string CheckList::STANDARDS_TEST = "standards_test";
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
/*
CheckList::~CheckList()
{
	delete[] os;
}
*/
void CheckList::startNewGroup(const string& groupName)
{
	counter = 1;
	os << endl << groupName << endl;
}

void CheckList::writeResult(const string& testCaseDesc, bool result)
{
	os << counter++ << "|" << testCaseDesc << "|" << (result ? "Ok" : 
		"Failed") << endl; }

}
}
}

