#ifndef SCAG_CONFIG_CONFIGEXCEPTION
#define SCAG_CONFIG_CONFIGEXCEPTION

#include <util/Exception.hpp>

namespace scag {
namespace config {

class ConfigException : public smsc::util::Exception
{
public:
/*	ConfigException(const char * const exceptionMessage)
		: Exception(exceptionMessage)
	{}*/
	ConfigException(const char * const exceptionMessage, ...) : smsc::util::Exception()
	{SMSC_UTIL_EX_FILL(exceptionMessage);}
};

}
}

namespace scag2 {
namespace config {
using scag::config::ConfigException;
}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
