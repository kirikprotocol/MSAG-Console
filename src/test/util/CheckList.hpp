#ifndef TEST_UTIL_CHECK_LIST
#define TEST_UTIL_CHECK_LIST

#include "core/synchronization/Mutex.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace util {

using std::map;
using std::set;
using std::string;
using std::vector;
using smsc::core::synchronization::Mutex;

/**
 * ������ � ���������� ���� �����.
 */
struct TestCase
{
	const string id;
	const string desc;
	int correct;
	int incorrect;
	set<int> errCodes;
	Mutex mutex;

	TestCase(const string _id, const string _desc)
		: id(_id), desc(_desc), correct(0), incorrect(0) {}
	Mutex& getMutex() { return mutex; }
};

/**
 * �������� ����� ��� ������ ����������� � checklist.
 */
class CheckList
{
	string name;
	typedef map<const string, TestCase*> TcMap;
	typedef vector<TestCase*> TcList;
	TcMap tcMap;
	TcList tcList;
	mutable Mutex mutex;

public:
	CheckList(const char* _name) : name(_name) {}
	virtual ~CheckList();

	/**
	 * ���� ���� id ������ ���� � ������� a.b.c...
	 * ����� ���������������� ���� ���� a.b.c, ����������� ������ ����
	 * ���������������� ����� ��������������� ���� ����� a � a.b.
	 * ���� ���� ���� � ����� id ��� ���������������, ������������ ������ ���� ����.
	 */
	TestCase* registerTc(const char* id, const char* desc);

	/**
	 * ���� ���� ���� id �����������������, �������� exception.
	 */
	TestCase* getTc(const char* id) const;

	void save(const char* fileName, bool printErrorCodes = true,
		bool printExecCount = true, bool printTcIds = true) const;
};

}
}
}

#endif /* TEST_UTIL_CHECK_LIST */

