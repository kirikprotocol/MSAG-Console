#ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
#define SMSC_UTIL_CONFIG_CONFIGEXCEPTION

namespace smsc {
namespace util {
namespace config {

class ConfigException
{
public:
	ConfigException(const char * const exceptionMessage) : message(exceptionMessage) {};

	const char * getMessage() {return message;};
private:
	const char * const message;
};

}
}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIGEXCEPTION
