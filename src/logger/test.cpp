#include "logger/Logger.h"
#include "logger/additional/ConfigReader.h"
#include "util/Properties.h"

#include <iostream>
#include <time.h>
#include "core/threads/Thread.hpp"

using namespace smsc::logger;
using namespace smsc::util;

void mySleep(int millisecs)
{
	struct timespec s;
	s.tv_sec = millisecs/1000;
	s.tv_nsec = (millisecs%1000)*1000*1000;
	nanosleep(&s, NULL);
}

void testLoggers(Logger** loggers, const size_t count, const Logger::LogLevel & logLevel)
{
	std::cerr << "***" << std::endl;
	for (int i = 0; i < count; i++)
		loggers[i]->log_(logLevel, "/// %s, %s\\\\\\", Logger::getLogLevel(logLevel), loggers[i]->getName());
}

class TestLogThread1 : public smsc::core::threads::Thread
{
private:
	Mutex mutex;
	bool isStopping;
public:
	virtual int Execute()
	{
		isStopping = false;
		Logger * loggersForTest[] = {
			Logger::getInstance("map"),
				Logger::getInstance("map.dialog"),
				Logger::getInstance("map.proxy"),
				Logger::getInstance("sms"),
				Logger::getInstance("sms.error"),
				Logger::getInstance("trace"),
				Logger::getInstance("����")
		};
		while (true) {
			{
				MutexGuard guard(mutex);
				if (isStopping)
					return 0;
			}
			std::cerr << "*********************" << std::endl;
			testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_DEBUG);
			testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_INFO);
			testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_WARN);
			testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_ERROR);
			testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_FATAL);
			testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_NOTSET);
			mySleep(1);
		}
	}
	void Stop()
	{
		MutexGuard guard(mutex);
		isStopping = true;
	}
};

class TestLogThread2 : public smsc::core::threads::Thread
{
private:
	Mutex mutex;
	bool isStopping;
public:
	virtual int Execute()
	{
		Logger::LogLevels levels[2];

		levels[0][""] = Logger::LEVEL_INFO;
		levels[0]["map"] = Logger::LEVEL_DEBUG;
		levels[0]["map.proxy"] = Logger::LEVEL_ERROR;

		levels[1][""] = Logger::LEVEL_WARN;
		levels[1]["map"] = Logger::LEVEL_INFO;
		levels[1]["map.dialog"] = Logger::LEVEL_DEBUG;
		levels[1]["map.proxy"] = Logger::LEVEL_FATAL;
		levels[1]["sms.error"] = Logger::LEVEL_ERROR;

		int i = 0;
		while (true) {
			{
				MutexGuard guard(mutex);
				if (isStopping)
					return 0;
			}
			Logger::setLogLevels(levels[i]);
			i = ++i%(sizeof(levels)/sizeof(levels[0]));
			mySleep(500);
		}
	}
	void Stop()
	{
		MutexGuard guard(mutex);
		isStopping = true;
	}
};

#ifdef SMSC_DEBUG
#endif //SMSC_DEBUG

int main()
{
#ifdef SMSC_DEBUG
	Logger::Init("logger.props");

	std::cout << "*********************" << std::endl;
	Logger::printDebugInfo();

	Logger * loggersForTest[] = {
		Logger::getInstance("map"),
		Logger::getInstance("map.dialog"),
		Logger::getInstance("map.proxy"),
		Logger::getInstance("sms"),
		Logger::getInstance("sms.error"),
		Logger::getInstance("trace"),
		Logger::getInstance("����")
	};

	std::cout << "*********************" << std::endl;
	Logger::printDebugInfo();

	{	/// test loggers levels
		std::cerr << "*********************" << std::endl;
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_DEBUG);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_INFO);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_WARN);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_ERROR);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_FATAL);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_NOTSET);
	}

	{	/// print log levels
		std::cout << "*********************" << std::endl;
		std::cout << "Log levels:" << std::endl;
		const Logger::LogLevels & levels = Logger::getLogLevels();
		char *k;
		Logger::LogLevel level;
		for (Logger::LogLevels::Iterator i = levels.getIterator(); i.Next(k, level); )
		{
			std::cout << "  " << k << "->" << Logger::getLogLevel(level) << std::endl;
		}
	}

	{	/// test setLogLevels
		Logger::LogLevels levels;
		levels[""] = Logger::LEVEL_INFO;
		levels["map"] = Logger::LEVEL_DEBUG;
		levels["map.proxy"] = Logger::LEVEL_ERROR;

		Logger::setLogLevels(levels);
	}

	std::cout << "*********************" << std::endl;
	Logger::printDebugInfo();

	{	/// test loggers levels
		std::cerr << "*********************" << std::endl;
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_DEBUG);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_INFO);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_WARN);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_ERROR);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_FATAL);
		testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_NOTSET);
	}

/*
	TestLogThread1 threads1[256];
	TestLogThread2 threads2[4];

	std::cout << "starting..." << std::endl;
	for (int i=0; i<sizeof(threads1)/sizeof(threads1[0]); i++)
	{
		threads1[i].Start();
	}
	for (int i=0; i<sizeof(threads2)/sizeof(threads2[0]); i++)
	{
		threads2[i].Start();
	}

	std::cout << "started, sleeping..." << std::endl;
	mySleep(10*60*1000);

	std::cout << "stopping..." << std::endl;
	for (int i=0; i<sizeof(threads1)/sizeof(threads1[0]); i++)
	{
		threads1[i].Stop();
		std::cout << "thread1 " << i << " Stop() called" << std::endl;
	}
	for (int i=0; i<sizeof(threads2)/sizeof(threads2[0]); i++)
	{
		threads2[i].Stop();
		std::cout << "thread2 " << i << " Stop() called" << std::endl;
	}
	std::cout << "stopped, waiting..." << std::endl;
	for (int i=0; i<sizeof(threads1)/sizeof(threads1[0]); i++)
	{
		threads1[i].WaitFor();
		std::cout << "thread1 " << i << " stopped" << std::endl;
	}
	for (int i=0; i<sizeof(threads2)/sizeof(threads2[0]); i++)
	{
		threads2[i].WaitFor();
		std::cout << "thread2 " << i << " stopped" << std::endl;
	}
	std::cout << "threads finished" << std::endl;
*/	
	std::cout << "shutdown" << std::endl;
	Logger::Shutdown();
	std::cout << "program done" << std::endl;

#else
	std::cout << "Recompile with DEBUG=YES please" << std::endl;
#endif //SMSC_DEBUG
}
