#ifndef TEST_UTIL_DATE_FORMATTER
#define TEST_UTIL_DATE_FORMATTER

#include <string>

namespace smsc {
namespace test {
namespace util {

using std::string;

class DateFormatter
{
	const string fmt;
	int count(string::const_iterator& it, char ch, int maxCount) const;
public:
	DateFormatter(const string& _fmt) : fmt(_fmt) {}
	const string format(time_t t) const;
};

}
}
}

#endif /* TEST_UTIL_DATE_FORMATTER */

