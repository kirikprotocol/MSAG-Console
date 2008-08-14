#ifndef SCAG_CONFIG_CONFIGEXCEPTION
#define SCAG_CONFIG_CONFIGEXCEPTION

#include <util/Exception.hpp>

namespace scag {
namespace config {

using smsc::util::Exception;

class ConfigException	: public Exception
{
public:
/*	ConfigException(const char * const exceptionMessage)
		: Exception(exceptionMessage)
	{}*/
	ConfigException(const char * const exceptionMessage, ...)
		: Exception()
	{SMSC_UTIL_EX_FILL(exceptionMessage);}
};

}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
