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
	{
		message = new char[strlen(exceptionMessage) +1];
		strcpy(message, exceptionMessage);
	};

	virtual ~ConfigException()
		throw ()
	{
		delete[] message;
	}

	const char * what() const
		throw()
	{return message;};

private:
	char * message;
};

}
}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
