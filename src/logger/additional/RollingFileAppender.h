#ifndef SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER
#define SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER

#include <stdio.h>

#include <logger/Appender.h>
#include <core/synchronization/Mutex.hpp>
#include <util/Properties.h>

namespace smsc {
namespace logger {

using namespace smsc::util;

class RollingFileAppender : public Appender
{
public:
	RollingFileAppender(const char * const name, const Properties & properties);

	virtual void log(const char logLevelName, const char * const category, const char * const message) throw();

private:
	smsc::core::synchronization::Mutex mutex;
	unsigned int maxBackupIndex;
	long maxFileSize;
	std::auto_ptr<const char> filename;
	FILE * file;
  long currentFilePos;

	void rollover() throw();
};

}
}

#endif //SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER
