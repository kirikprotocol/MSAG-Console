#ifndef TEST_DBSME_DATE_FORMATTER
#define TEST_DBSME_DATE_FORMATTER

#include <string>
#include <ostream>
#include <sstream>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using std::ostream;
using std::ostringstream;

class DateFormatter
{
	const string fmt;
	void processToken(ostream& res, const string& token, const tm& lt) const;
public:
	DateFormatter(const string& _fmt) : fmt(_fmt) {}
	const string format(time_t t) const;
};

}
}
}

#endif /* TEST_DBSME_DATE_FORMATTER */

