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
 * ƒанные и результаты тест кейса.
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
 * ќсновной класс дл€ записи результатов в checklist.
 */
class CheckList
{
	string name;
	string fileName;
	typedef map<const string, TestCase*> TcMap;
	typedef vector<TestCase*> TcList;
	TcMap tcMap;
	TcList tcList;
	mutable Mutex mutex;

public:
	CheckList(const char* _name, const char* _fileName)
		: name(_name), fileName(_fileName) {}
	virtual ~CheckList();

	/**
	 * “ест кейс id должен быть в формате a.b.c...
	 * „тобы зарегистрировать тест кейс a.b.c, об€зательно должны быть
	 * зарегистрированы более высокоуровненые тест кейсы a и a.b.
	 * ≈сли тест кейс с таким id уже зарегистрирован, возвращаетс€ старый тест кейс.
	 */
	TestCase* registerTc(const char* id, const char* desc);

	/**
	 * ≈сли тест кейс id незарегистрирован, кидаетс€ exception.
	 */
	TestCase* getTc(const char* id) const;

	void save(bool printErrorCodes = true,
		bool printExecCount = true, bool printTcIds = true) const;
};

}
}
}

#endif /* TEST_UTIL_CHECK_LIST */

