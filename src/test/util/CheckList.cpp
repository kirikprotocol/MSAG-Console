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

void CheckList::startNewGroup(const string& groupName, const string& packageName)
{
	counter = 1;
	os << endl << "|" << groupName << endl;
	os << "|Покрытие кода для " << packageName << "|" << endl;
	os << "№|Тест|Результат|Стек" << endl;
}

void CheckList::writeResult(const string& testCaseDesc, TCResult& result)
{
	os << counter++ << "|" << testCaseDesc << "|";
	os << (result.value() ? "Да" : "Нет") << "|";
	result.print(os);
	os << endl;
	os.flush();
}

void CheckList::writeResult(TCResultFilter& resultFilter)
{
	for (TCResultFilter::TCMap::iterator it = resultFilter.resmap.begin();
		 it != resultFilter.resmap.end(); it++)
	{
		TCResultFilter::TCValue& tcVal = *it->second;
		os << counter++ << "|" << tcVal.description << "|";
		if (!tcVal.used)
		{
			os << "-|";
		}
		else if (tcVal.tcStacks.size() == 0)
		{
			os << "Да|";
		}
		else
		{
			os << "Нет|";
			for (int i = 0; i < tcVal.tcStacks.size(); i++)
			{
				if (i > 0)
				{
					os << ",";
				}
				TCResultStack& stack = *tcVal.tcStacks[i];
				for (int j = 0; j < stack.size(); j++)
				{
					if (j > 0)
					{
						os << "->";
					}
					TCResult& res = *stack[j];
					os << res.getId() << "(" << res.getChoice() << ")";
				}
			}
		}
		os << endl;
	}
	os.flush();
}

}
}
}

