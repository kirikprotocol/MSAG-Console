#ifndef TEST_UTIL_CHECK_LIST
#define TEST_UTIL_CHECK_LIST

#include "AutoSync.hpp"
#include "core/synchronization/Mutex.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>

namespace smsc {
namespace test {
namespace util {

using std::map;
using std::set;
using std::string;
using std::vector;
using std::pair;
using smsc::core::synchronization::Mutex;

//макросы для работы с тест кейсами
#define __decl_tc__ \
	TestCase* tc = NULL; \
	bool isOk = true;

#define __tc__(tcId) \
	__require__(isOk); \
	if (chkList) { \
		static TestCase* _tc = chkList->getTc(tcId); \
		tc = _tc; \
	}

#define __tc_ok__ \
	if (chkList) { \
		__require__(tc); \
		__trace2__("tc = %s: ok", tc->id.c_str()); \
		SyncTestCase _tc(tc); \
		_tc->correct++; \
	} \
	isOk = true;

#define __tc_ok_cond__ \
	if (chkList && isOk) { \
		__require__(tc); \
		__trace2__("tc = %s: ok", tc->id.c_str()); \
		SyncTestCase _tc(tc); \
		_tc->correct++; \
	} \
	isOk = true;

#define __tc_fail__(errCode) \
	if (chkList) { \
		__require__(tc); \
		__trace2__("tc = %s: err = %d", tc->id.c_str(), errCode); \
		isOk = false; \
		SyncTestCase _tc(tc); \
		_tc->incorrect++; \
		_tc->errCodes.insert(errCode); \
	}

#define __tc_fail2__(errList, shift) \
	{ vector<int> tmp(errList); \
	if (chkList && tmp.size()) { \
		__require__(tc); \
		if (shift) { \
			transform(tmp.begin(), tmp.end(), tmp.begin(), bind2nd(plus<int>(), shift)); \
		} \
		ostringstream s; \
		copy(tmp.begin(), tmp.end(), ostream_iterator<int>(s, ",")); \
		__trace2__("tc = %s: err = %s", tc->id.c_str(), s.str().c_str()); \
		isOk = false; \
		SyncTestCase _tc(tc); \
		_tc->incorrect++; \
		_tc->errCodes.insert(tmp.begin(), tmp.end()); \
	}}

#define __decl_tc12__ \
	TestCase* tc1 = NULL; \
	TestCase* tc2 = NULL; \
	bool isOk = true;

#define __tc1__(tcId) \
	__require__(isOk); \
	if (chkList) { \
		static TestCase* _tc = chkList->getTc(tcId); \
		tc1 = _tc; \
	}

#define __tc2__(tcId) \
	__require__(isOk); \
	if (chkList) { \
		static TestCase* _tc = chkList->getTc(tcId); \
		tc2 = _tc; \
	}

#define __tc12_ok__ \
	if (chkList) { \
		__require__(tc1 && tc2 && tc1 != tc2); \
		__trace2__("tc = %s, tc = %s: ok", tc1->id.c_str(), tc2->id.c_str()); \
		SyncTestCase _tc1(tc1); \
		_tc1->correct++; \
		SyncTestCase _tc2(tc2); \
		_tc2->correct++; \
	} \
	isOk = true;

#define __tc12_ok_cond__ \
	if (chkList && isOk) { \
		__require__(tc1 && tc2 && tc1 != tc2); \
		__trace2__("tc = %s, tc = %s: ok", tc1->id.c_str(), tc2->id.c_str()); \
		SyncTestCase _tc1(tc1); \
		_tc1->correct++; \
		SyncTestCase _tc2(tc2); \
		_tc2->correct++; \
	} \
	isOk = true;

#define __tc12_fail__(errCode) \
	if (chkList) { \
		__require__(tc1 && tc2 && tc1 != tc2); \
		__trace2__("tc = %s, tc = %s: err = %d", tc1->id.c_str(), tc2->id.c_str(), errCode); \
		isOk = false; \
		SyncTestCase _tc1(tc1); \
		_tc1->incorrect++; \
		_tc1->errCodes.insert(errCode); \
		SyncTestCase _tc2(tc2); \
		_tc2->incorrect++; \
		_tc2->errCodes.insert(errCode); \
	}

#define __tc12_fail2__(errList, shift) \
	{ vector<int> tmp(errList); \
	if (chkList && tmp.size()) { \
		__require__(tc1 && tc2 && tc1 != tc2); \
		if (shift) { \
			transform(tmp.begin(), tmp.end(), tmp.begin(), bind2nd(plus<int>(), shift)); \
		} \
		ostringstream s; \
		copy(tmp.begin(), tmp.end(), ostream_iterator<int>(s, ",")); \
		__trace2__("tc = %s, tc = %s: err = %s", tc1->id.c_str(), tc2->id.c_str(), s.str().c_str()); \
		isOk = false; \
		SyncTestCase _tc1(tc1); \
		_tc1->incorrect++; \
		_tc1->errCodes.insert(tmp.begin(), tmp.end()); \
		SyncTestCase _tc2(tc2); \
		_tc2->incorrect++; \
		_tc2->errCodes.insert(tmp.begin(), tmp.end()); \
	}}

/**
 * Данные и результаты тест кейса.
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

typedef auto_sync<TestCase> SyncTestCase;

/**
 * Основной класс для записи результатов в checklist.
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
	 * Тест кейс id должен быть в формате a.b.c...
	 * Чтобы зарегистрировать тест кейс a.b.c, обязательно должны быть
	 * зарегистрированы более высокоуровненые тест кейсы a и a.b.
	 * Тест кейс с таким id нельзя зарегистрировать дважды.
	 */
	TestCase* registerTc(const char* id, const char* desc);

	/**
	 * Убрать вывод тест кейса в checklist.
	 */
	void hideTc(const string& id);

	/**
	 * Если тест кейс id незарегистрирован, кидается exception.
	 */
	TestCase* getTc(const char* id) const;

	/**
	 * Обнуление счетчиков и ошибок.
	 */
	void reset();

	void save(bool printErrorCodes = true,
		bool printExecCount = true, bool printTcIds = true) const;

	void saveHtml(bool printErrorCodes = true,
		bool printExecCount = true, bool printTcIds = true) const;

private:
	int getTcMag(const TestCase* tc) const;
	pair<int, int> CheckList::getCounts(const TestCase* tc) const;
	const string toHtmlString(const string& str) const;
};

}
}
}

#endif /* TEST_UTIL_CHECK_LIST */

