#ifndef TEST_UTIL_BASE_TEST_CASES
#define TEST_UTIL_BASE_TEST_CASES

#include "test/util/Util.hpp"
#include "util/Logger.h"
#include <iostream>
#include <sstream>

namespace smsc {
namespace test {
namespace util {

using namespace std;
using log4cpp::Category;
using smsc::test::util::TCResult;

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
	void debug(const TCResult* res);
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

inline void BaseTestCases::debug(const TCResult* res)
{
	if (res)
	{
		ostringstream os;
		os << *res << endl;
		getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
	}
}

}
}
}

#endif /* TEST_UTIL_BASE_TEST_CASES */

