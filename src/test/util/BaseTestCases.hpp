#ifndef TEST_UTIL_BASE_TEST_CASES
#define TEST_UTIL_BASE_TEST_CASES

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

