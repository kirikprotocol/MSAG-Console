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

int CheckList::getTcMag(const TestCase* tc) const
{
	__require__(tc);
	return count(tc->id.begin(), tc->id.end(), '.');
}

pair<int, int> CheckList::getCounts(const TestCase* tc) const
{
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
	return make_pair(correct, incorrect);
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
		int mag = getTcMag(tc);
		pair<int, int> counts = getCounts(tc);
		os << cnt++ << "|" << string(4 * mag, ' ').c_str() << tc->desc << "|" <<
			(counts.second ? "Нет" : (counts.first ? "Да" : "-"));
		if (printExecCount)
		{
			if (counts.second)
			{
				os << "|" << counts.first << "/" << (counts.first + counts.second);
			}
			else
			{
				os << "|" << counts.first;
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

void CheckList::saveHtml(bool printErrorCodes,
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
	ofstream os((fileName + ".html").c_str());
	time_t lt = time(NULL);
	tm t;
	char buf[30];
	os << "<html>" << endl;
	os << "<head><title>" << name << "</title></head>" << endl;
	os << "<body><table width=100%>" << endl;
	os << "<tr>" << name << "</tr>" << endl;
	os << "<tr>Версия:</tr>" << endl;
	os << "<tr>Время: " << asctime_r(localtime_r(&lt, &t), buf) << "</tr>" << endl;
	os << "</table>" << endl;
	os << "<table width=\"100%\" style=\"font-size: 12px;\" cellspacing=\"0\" cellpadding=\"2\" border=\"1\">" << endl;
	os << "<tr align=\"center\">" << endl;
	os << "<td>№</td>" << endl;
	os << "<td>Тест кейс</td>" << endl;
	os << "<td>Результат</td>" << endl;
	if (printExecCount)
	{
		os << "<td>Счетчики</td>" << endl;
	}
	if (printErrorCodes)
	{
		os << "<td>Коды ошибок</td>" << endl;
	}
	if (printTcIds)
	{
		os << "<td>tcId</td>" << endl;
	}
	os << "</tr>" << endl;
	int cnt = 1;
	for (TcList::const_iterator it = tcList.begin(); it != tcList.end(); it++)
	{
		TestCase* tc = *it;
		int mag = getTcMag(tc);
		pair<int, int> counts = getCounts(tc);
		os << "<tr>" << endl;
		os << "<td align=\"center\">" << cnt++ << "</td>" << endl;
		os << "<td align=\"left\" style=\"padding-left: " << (20 * mag + 2) << ";\">" << tc->desc << "</td>" <<	endl;
		os << "<td align=\"center\">" << (counts.second ? "Нет" : (counts.first ? "Да" : "-")) << "</td>" << endl;
		if (printExecCount)
		{
			if (counts.second)
			{
				os << "<td align=\"right\">" << counts.first << "/" << (counts.first + counts.second) << "</td>" << endl;
			}
			else
			{
				os << "<td align=\"right\">" << counts.first << "</td>" << endl;
			}
		}
		if (printErrorCodes)
		{
			os << "<td align=\"left\">";
			if (tc->errCodes.size())
			{
				copy(tc->errCodes.begin(), tc->errCodes.end(), ostream_iterator<int>(os, ", "));
			}
			else
			{
				os << "&nbsp;";
			}
			os << "</td>" << endl;
		}
		if (printTcIds)
		{
			os << "<td align=\"left\">" << tc->id << "</td>" << endl;
		}
		os << "</tr>" << endl;
	}
	os << "</table></body>" << endl;
	os << "</html>" << endl;
}

}
}
}

