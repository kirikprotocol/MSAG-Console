#include "CheckList.hpp"
#include "util/debug.h"
#include <algorithm>
#include <fstream>

namespace smsc {
namespace test {
namespace util {

using smsc::core::synchronization::MutexGuard;
using namespace std;

CheckList::~CheckList()
{
	for (TcList::iterator it = tcList.begin(); it != tcList.end(); it++)
	{
		delete *it;
	}
}

TestCase* CheckList::registerTc(const char* id, const char* desc)
{
	MutexGuard mguard(mutex);
	TcMap::iterator it = tcMap.find(id);
	__require__(it == tcMap.end());
	TestCase* tc = new TestCase(id, desc);
	tcMap[id] = tc;
	tcList.push_back(tc);
	return tc;
}

TestCase* CheckList::getTc(const char* id) const
{
	TcMap::const_iterator it = tcMap.find(id);
	__require__(it != tcMap.end());
	return it->second;
}

void CheckList::save(bool printErrorCodes,
		bool printExecCount, bool printTcIds) const
{
	MutexGuard mguard(mutex);
	//упорядочить тест кейсы
	/*
	struct TcComparator
	{
		bool operator< (const TestCase& tc1, const TestCase& tc2)
		{
			return tc1.id < tc2.id;
		}
	};
	stable_sort(tcList.begin(), tcList.end(), TcComparator());
	*/
	//записать check list в файл
	ofstream os(fileName.c_str());
	time_t lt = time(NULL);
	tm t;
	char buf[30];
	os << name << endl;
	os << "Версия: " << endl;
	os << "Время: " << asctime_r(localtime_r(&lt, &t), buf) << endl;
	os << "№|Тест кейс|Результат" <<
		(printExecCount ? "|Счетчики" : "") <<
		(printErrorCodes ? "|Коды ошибок" : "") <<
		(printTcIds ? "|tcId" : "") << endl;
	int cnt = 1;
	for (TcList::const_iterator it = tcList.begin(); it != tcList.end(); it++)
	{
		TestCase* tc = *it;
		int mag = count(tc->id.begin(), tc->id.end(), '.');
		TcMap::const_iterator itSelf = tcMap.find(tc->id);
		__require__(itSelf != tcMap.end());
		int correct = itSelf->second->correct;
		int incorrect = itSelf->second->incorrect;
		TcMap::const_iterator itSub = tcMap.lower_bound(tc->id + '.');
		TcMap::const_iterator itSubEnd = tcMap.lower_bound(tc->id + '/');
		for (; itSub != itSubEnd; itSub++)
		{
			correct += itSub->second->correct;
			incorrect += itSub->second->incorrect;
		}
		os << cnt++ << "|" << string(4 * mag, ' ').c_str() << tc->desc << "|" <<
			(incorrect ? "Нет" : (correct ? "Да" : "-"));
		if (printExecCount)
		{
			if (incorrect)
			{
				os << "|" << correct << "/" << (correct + incorrect);
			}
			else
			{
				os << "|" << correct;
			}
		}
		if (printErrorCodes)
		{
			os << "|";
			copy(tc->errCodes.begin(), tc->errCodes.end(), ostream_iterator<int>(os, ", "));
		}
		if (printTcIds)
		{
			os << "|" << tc->id;
		}
		os << endl;
	}
}

}
}
}

