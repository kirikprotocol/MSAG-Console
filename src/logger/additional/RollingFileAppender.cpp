#include "logger/additional/RollingFileAppender.h"

#include <time.h>
#include <pthread.h>

#include "util/cstrings.h"

namespace smsc {
namespace logger {

using namespace smsc::util;

void rolloverFiles(const char * const filename, unsigned int maxBackupIndex)
{
	const size_t max_filename_length = strlen(filename)+16;
	std::auto_ptr<char> buffer(new char[max_filename_length+1]);
	snprintf(buffer.get(), max_filename_length, "%s.%u", filename, maxBackupIndex);
	buffer.get()[max_filename_length] = 0;
	remove(buffer.get());

	// Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
	for(int i=maxBackupIndex - 1; i >= 1; i--) {
		std::auto_ptr<char> source(new char[max_filename_length+1]);
		std::auto_ptr<char> target(new char[max_filename_length+1]);
		
		snprintf(source.get(), max_filename_length, "%s.%u", filename, i);
		snprintf(target.get(), max_filename_length, "%s.%u", filename, i+1);

		source.get()[max_filename_length] = 0;
		target.get()[max_filename_length] = 0;

		rename(source.get(), target.get());
	}
} 


RollingFileAppender::RollingFileAppender(const char * const _name, const Properties & properties)
	:Appender(_name)
{
	maxFileSize = 1*1024*1024;
	maxBackupIndex = 5;

	if(properties.Exists("maxsize")) {
		const char * const tmp = properties["maxsize"];
		const size_t tmpLen = strlen(tmp);
		maxFileSize = atoi(tmp);
		if (tmpLen > 2)
		{
			const char * const suffix = tmp + (tmpLen -2);
			if (strcasecmp("mb", suffix) == 0)
			{
				maxFileSize *= (1024 * 1024); // convert to megabytes
			}
			else if (strcasecmp("kb", suffix) == 0)
			{
				maxFileSize *= 1024; // convert to kilobytes
			}
		}
	}

	if (properties.Exists("maxindex")) 
		maxBackupIndex = atoi(properties["maxindex"]);

	if (properties.Exists("name"))
		filename.reset(cStringCopy(properties["name"]));
	else
		filename.reset(cStringCopy("smsc.log"));

	file = fopen(filename.get(), "a");
}

void RollingFileAppender::rollover() throw()
{
    // If maxBackups <= 0, then there is no file renaming to be done.
    if(maxBackupIndex > 0) {
		rolloverFiles(filename.get(), maxBackupIndex);

        // Close the current file
        fclose(file);
		file = NULL;

        // Rename fileName to fileName.1
		const size_t max_filename_length = strlen(filename.get())+16;
		std::auto_ptr<char> target(new char[max_filename_length+1]);
		snprintf(target.get(), max_filename_length, "%s.1", filename.get());
		target.get()[max_filename_length] = 0;

		rename(filename.get(), target.get());

        // Open a new file
		file = fopen(filename.get(), "w");
    }
    else {
		fclose(file);
		file = fopen(filename.get(), "w");
    }
}

void RollingFileAppender::log(const char logLevelName, const char * const category, const char * const message) throw()
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
	fprintf(file != NULL ? file : stderr, "%c %s,%3.3u %3.3u % 10.10s: %s\n", logLevelName, timeStr, tp.tv_usec/1000, ::pthread_self(), category, message);
  fflush(file);
	if (file != NULL && ftell(file) > maxFileSize)
		rollover();
}

}
}
