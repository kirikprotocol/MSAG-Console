#ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
#define SMSC_UTIL_CONFIG_CONFIGEXCEPTION

#include <exception>

namespace smsc {
namespace util {
namespace config {

class ConfigException	: public std::exception
{
public:
	ConfigException(const char * const exceptionMessage)
		throw ()
		: message(exceptionMessage)
	{};

	const char * what() const
		throw()
	{return message;};

private:
	const char * const message;
};

}
}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
