#include "MonitoringData.h"

#include <string>

namespace smsc {
namespace admin {
namespace service {

char * MonitoringData::getText() const
{
	std::string result("<monitoring ");
	char * name;
	int32_t value;
	for (MonitoringData::Iterator i = this->getIterator(); i.Next(name, value); )
	{
		result += name;
		result += "=\"";
		char tmp[33];
		sprintf(tmp, "%li", (long)value);
		result += tmp;
		result += "\" ";
	}
	result += "/>";
	char *r = new char[result.length()+1];
	strcpy(r, result.c_str());
	return r;
}

}
}
}
