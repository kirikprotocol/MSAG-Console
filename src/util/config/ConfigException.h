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
	{}

	~ConfigException()
		throw ()
	{}

	const char * what() const
		throw()
	{return message.c_str();}

private:
	std::string message;
};

}
}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
