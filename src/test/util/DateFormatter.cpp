#include "DateFormatter.hpp"
#include <sstream>

namespace smsc {
namespace test {
namespace util {

using std::ostringstream;

int DateFormatter::count(string::const_iterator& it, char ch, int maxCount) const
{
	int count = 0;
	for (; it != fmt.end() && *it == ch && count < maxCount; it++)
	{
		count++;
	}
	it--;
	return count;
}

const string DateFormatter::format(time_t t) const
{
	//for (int i = 0; i < fmt.length(); i++)
	ostringstream res;
	tm lt;
	localtime_r(&t, &lt);
	string::const_iterator it_begin = fmt.begin();
	char str[16];
	int hour;
	for (string::const_iterator it = fmt.begin(); it != fmt.end(); it++)
	{
		switch (*it)
		{
			case 'w': //Abbreviated weekday name
				strftime(str, sizeof(str), "%a", &lt);
				res << str;
				break;
			case 'W': //Full weekday name
				strftime(str, sizeof(str), "%A", &lt);
				res << str;
				break;
			case 'd':
				switch (count(it, 'd', 2))
				{
					case 1:
						res << lt.tm_mday;
						break;
					case 2:
						res << (lt.tm_mday < 10 ? "0" : "");
						res << lt.tm_mday;
						break;
				}
				break;
			case 'M':
				switch (count(it, 'M', 4))
				{
					case 1:
						res << (lt.tm_mon + 1);
						break;
					case 2:
						res << (lt.tm_mon < 9 ? "0" : "");
						res << (lt.tm_mon + 1);
						break;
					case 3: //Abbreviated month name
						strftime(str, sizeof(str), "%b", &lt);
						res << str;
						break;
					case 4: //Full month name
						strftime(str, sizeof(str), "%B", &lt);
						res << str;
						break;
				}
				break;
			case 'y':
				switch (count(it, 'y', 4))
				{
					case 2: //Year without century
						res << (lt.tm_year % 100 < 10 ? "0" : "");
						res << (lt.tm_year % 100);
						break;
					case 4:
						res << lt.tm_year + 1900;
						break;
					default:
						return "";
				}
				break;
			case 'h':
				strftime(str, sizeof(str), "%I", &lt); //1-12
				sscanf(str, "%d", &hour);
				switch (count(it, 'h', 2))
				{
					case 1:
						res << hour; 
						break;
					case 2:
						res << (hour < 10 ? "0" : "");
						res << hour;
						break;
				}
				break;
			case 'H':
				switch (count(it, 'H', 2))
				{
					case 1:
						res << lt.tm_hour;
						break;
					case 2:
						res << (lt.tm_hour < 10 ? "0" : "");
						res << lt.tm_hour;
						break;
				}
				break;
			case 'm':
				switch (count(it, 'm', 2))
				{
					case 1:
						res << lt.tm_min;
						break;
					case 2:
						res << (lt.tm_min < 10 ? "0" : "");
						res << lt.tm_min;
						break;
				}
				break;
			case 's':
				switch (count(it, 's', 2))
				{
					case 1:
						res << lt.tm_sec;
						break;
					case 2:
						res << (lt.tm_sec < 10 ? "0" : "");
						res << lt.tm_sec;
						break;
				}
				break;
			case 't': //Current locale's A.M./P.M. indicator
				strftime(str, sizeof(str), "%p", &lt);
				res << str;
				break;
			default:
				if (!isalnum(*it))
				{
					res << *it;
				}
				else
				{
					return "";
				}
		}
	}
	return res.str();
}

}
}
}

