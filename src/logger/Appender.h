#ifndef SMSC_LOGGER_APPENDER_HEADER
#define SMSC_LOGGER_APPENDER_HEADER

#include <memory>

namespace smsc {
namespace logger {

class Appender 
{
protected:
	std::auto_ptr<const char> name;
public:
	Appender(const char * const name);
	virtual ~Appender() {};
	virtual void log(const char logLevelName, const char * const category, const char * const message) throw() = 0;

	inline const char * const getName() const throw() { return name.get(); }
};

}
}

#endif //SMSC_LOGGER_APPENDER_HEADER
