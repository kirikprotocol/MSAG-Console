#ifndef TEST_DBSME_NUMBER_FORMATTER
#define TEST_DBSME_NUMBER_FORMATTER

#include <string>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;

struct NumberFormatter
{
	virtual const string format(double val) const = NULL;
};

class FloatFormatter : public NumberFormatter
{
	char fmt[16];
public:
	FloatFormatter(int precision)
	{
		__require__(precision >= 0);
		sprintf(fmt, "%%.%dlf", precision);
	}
	virtual const string format(double val) const
	{
		char res[32];
		sprintf(res, fmt, val);
		return res;
	}
};

class DoubleFormatter : public NumberFormatter
{
	char fmt[16];
public:
	DoubleFormatter(int digits, bool exponent)
	{
		__require__(digits > 0);
		if (exponent)
		{
			sprintf(fmt, "%%.%dle", digits - 1);
		}
		else
		{
			sprintf(fmt, "%%.%dlg", digits);
		}
	}
	virtual const string format(double val) const
	{
		char res[32];
		sprintf(res, fmt, val);
		return res;
	}
};

}
}
}

#endif /* TEST_DBSME_NUMBER_FORMATTER */

