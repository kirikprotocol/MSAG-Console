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
	if (it == tcMap.end())
	{
		TestCase* tc = new TestCase(id, desc);
		tcMap[id] = tc;
		tcList.push_back(tc);
		return tc;
	}
	else
	{
		return it->second;
	}
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
	time_t t = time(NULL);
	os << name << endl;
	os << "Версия: " << endl;
	os << "Время: " << asctime(localtime(&t)) << endl;
	os << "№|Тест кейс|Результат" <<
		(printExecCount ? "|Счетчики" : "") <<
		(printErrorCodes ? "|Коды ошибок" : "") <<
		(printTcIds ? "|tcId" : "") << endl;
	int cnt = 1;
	for (TcList::const_iterator it = tcList.begin(); it != tcList.end(); it++)
	{
		TestCase* tc = *it;
		int mag = count(tc->id.begin(), tc->id.end(), '.');
		int correct = 0, incorrect = 0;
		for (TcMap::const_iterator it2 = tcMap.find(tc->id); it2 != tcMap.end() &&
			!it2->first.compare(0, tc->id.length(), tc->id); it2++)
		{
			correct += it2->second->correct;
			incorrect += it2->second->incorrect;
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

