#include "logger/additional/StderrAppender.h"

#include <stdio.h>
#include <time.h>
#include <pthread.h>

namespace smsc {
namespace logger {

using namespace smsc::util;

StderrAppender::StderrAppender(const char * const name)
	:Appender(name)
{
}

void StderrAppender::log(const char logLevelName, const char * const category, const char * const message) throw()
{
	smsc::core::synchronization::MutexGuard guard(mutex);
	//D dd-mm hh:mm:ss,sss TTT CatLast___:message
	::timeval tp;
    ::gettimeofday(&tp, 0);
	::tm lcltm;
	::localtime_r(&tp.tv_sec, &lcltm);
	char timeStr[32];
	const size_t timeStrLength = ::strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m %H:%M:%S", &lcltm);
	timeStr[timeStrLength] = 0;
	fprintf(stderr, "%c %s,%3.3u %3.3u % 10.10s: %s\n", logLevelName, timeStr, tp.tv_usec/1000, ::pthread_self(), category, message);
}

}
}
