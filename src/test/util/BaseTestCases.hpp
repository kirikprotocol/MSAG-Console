#ifndef TEST_UTIL_BASE_TEST_CASES
#define TEST_UTIL_BASE_TEST_CASES

#include "AutoSync.hpp"
#include "CheckList.hpp"
#include "Util.hpp"
#include "util/Logger.h"
#include "core/threads/Thread.hpp"
#include <iostream>
#include <sstream>

namespace smsc {
namespace test {
namespace util {

using namespace std;
using log4cpp::Category;

typedef auto_sync<TestCase> SyncTestCase;

//макросы для работы с тест кейсами
#define __decl_tc__ \
	TestCase* tc = NULL; \
	bool isOk = true;

#define __tc__(tcId) \
	if (chkList) { tc = chkList->getTc(tcId); }

#define __tc_ok__ \
	if (chkList) { \
		__require__(tc); \
		__trace2__("%s: ok", tc->id.c_str()); \
		SyncTestCase _tc(tc); \
		_tc->correct++; \
	}

#define __tc_ok_cond__ \
	if (chkList && isOk) { \
		__require__(tc); \
		__trace2__("%s: ok", tc->id.c_str()); \
		SyncTestCase _tc(tc); \
		_tc->correct++; \
	}

#define __tc_fail__(errCode) \
	if (chkList) { \
		__require__(tc); \
		__trace2__("%s: err = %d", tc->id.c_str(), errCode); \
		isOk = false; \
		SyncTestCase _tc(tc); \
		_tc->incorrect++; \
		_tc->errCodes.insert(errCode); \
	}

#define __tc_fail2__(errList) \
	if (chkList && errList.size()) { \
		__require__(tc); \
		ostringstream s; \
		copy(errList.begin(), errList.end(), ostream_iterator<int>(s, ", ")); \
		__trace2__("%s: err = %s", tc->id.c_str(), s.str().c_str()); \
		isOk = false; \
		SyncTestCase _tc(tc); \
		_tc->incorrect++; \
		_tc->errCodes.insert(errList.begin(), errList.end()); \
	}

/**
 * Этот класс является базовым для все тест кейс имплементаций.
 * 
 * @author bryz
 */
class BaseTestCases
{
public:
	BaseTestCases();
	virtual ~BaseTestCases() {}

protected:
	virtual Category& getLog() = NULL;
	void error();
};

//BaseTestCases
inline BaseTestCases::BaseTestCases()
{
	init_rand();
}

inline void BaseTestCases::error()
{
	try
	{
		throw;
	}
	catch(exception& e)
	{
		getLog().error("[%d]\t%s", thr_self(), e.what());
	}
	catch(...)
	{
	}
}

}
}
}

#endif /* TEST_UTIL_BASE_TEST_CASES */

