#ifndef TEST_UTIL_BASE_TEST_CASES
#define TEST_UTIL_BASE_TEST_CASES

#include "CheckList.hpp"
#include "Util.hpp"
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include <iostream>
#include <sstream>

namespace smsc {
namespace test {
namespace util {

using namespace std;
using log4cpp::Category;

struct TCException : public exception
{
	const string msg;
	TCException() {}
	TCException(const char* str) : msg(str) {}
	~TCException() throw() {}
    virtual const char* what() const throw() { return msg.c_str(); }
};

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
		__warning2__("%s", e.what());
		getLog().error("[%d]\t%s", thr_self(), e.what());
	}
	catch(const char* s)
	{
		__warning2__("%s", s);
		getLog().error("[%d]\t%s", thr_self(), s);
	}
	catch(...)
	{
		__warning__("Unknown exception");
	}
}

}
}
}

#endif /* TEST_UTIL_BASE_TEST_CASES */

