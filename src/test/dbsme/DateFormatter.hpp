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

void DateFormatter::processToken(ostream& res, const string& token, const tm& lt) const
{
	//Abbreviated weekday name
	if (token == "w")
	{
		char str[16];
		strftime(str, sizeof(str), "%a", &lt);
		res << str;
	}
	//Full weekday name
	else if (token == "W")
	{
		char str[16];
		strftime(str, sizeof(str), "%A", &lt);
		res << str;
	}
	else if (token == "d")
	{
		res << lt.tm_mday;
	}
	else if (token == "dd")
	{
		res << (lt.tm_mday < 10 ? "0" : "");
		res << lt.tm_mday;
	}
	else if (token == "M")
	{
		res << (lt.tm_mon + 1);
	}
	else if (token == "MM")
	{
		res << (lt.tm_mon < 9 ? "0" : "");
		res << (lt.tm_mon + 1);
	}
	//Abbreviated month name
	else if (token == "MMM")
	{
		char str[16];
		strftime(str, sizeof(str), "%b", &lt);
		res << str;
	}
	//Full month name
	else if (token == "MMMM")
	{
		char str[16];
		strftime(str, sizeof(str), "%B", &lt);
		res << str;
	}
	//Year without century
	else if (token == "yy")
	{
		res << (lt.tm_year % 100 < 10 ? "0" : "");
		res << (lt.tm_year % 100);
	}
	else if (token == "yyyy")
	{
		res << lt.tm_year + 1900;
	}
	else if (token == "h")
	{
		res << lt.tm_hour;
	}
	else if (token == "hh")
	{
		res << (lt.tm_hour < 10 ? "0" : "");
		res << lt.tm_hour;
	}
	else if (token == "m")
	{
		res << lt.tm_min;
	}
	else if (token == "mm")
	{
		res << (lt.tm_min < 10 ? "0" : "");
		res << lt.tm_min;
	}
	else if (token == "s")
	{
		res << lt.tm_sec;
	}
	else if (token == "ss")
	{
		res << (lt.tm_sec < 10 ? "0" : "");
		res << lt.tm_sec;
	}
	//Current locale's A.M./P.M. indicator
	else if (token == "t")
	{
		char str[8];
		strftime(str, sizeof(str), "%p", &lt);
		res << str;
	}
	else
	{
		res << token;
	}
}

const string DateFormatter::format(time_t t) const
{
	//for (int i = 0; i < fmt.length(); i++)
	ostringstream res;
	tm lt;
	localtime_r(&t, &lt);
	string::const_iterator it_begin = fmt.begin();
	for (string::const_iterator it = fmt.begin(); it != fmt.end(); it++)
	{
		if (!isalpha(*it))
		{
			string token(it_begin, it);
			//cout << "token: " << token << endl;
			processToken(res, token, lt);
			res << *it;
			it_begin = it;
			it_begin++;
		}
	}
	string token(it_begin, fmt.end());
	//cout << "token: " << token << endl;
	processToken(res, token, lt);
	return res.str();
}

}
}
}

#endif /* TEST_DBSME_DATE_FORMATTER */

