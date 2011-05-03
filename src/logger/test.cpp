#include "logger/Logger.h"
#include "logger/additional/ConfigReader.h"
#include "util/Properties.h"
#include "core/synchronization/Mutex.hpp"

#include <iostream>
#include <time.h>
#include <sys/time.h>
#include "core/threads/Thread.hpp"

using namespace smsc::logger;
using namespace smsc::util;
using namespace smsc::core::synchronization;

unsigned long counterAll = 0;
unsigned long counterLogged = 0;
Mutex countersMutex;

#ifdef CHECKCONTENTION
namespace smsc {
namespace core {
namespace synchronization {
unsigned Mutex::contentionLimit = 200000;

void Mutex::reportContention( const char* from,
                              unsigned    howlong,
                              unsigned    oldcount ) const throw()
{
    fprintf(stderr,"%s @%s contented by %u@%s for %u usec count=%u/%+d\n",
            what, from ? from:"unk",
            unsigned(wasid),
            wasfrom?wasfrom:"unk",
            howlong,lockCount,
            int(lockCount-oldcount));
}

}
}
}
#endif

void mySleep(int millisecs)
{
  struct timespec s;
  s.tv_sec = millisecs/1000;
  s.tv_nsec = (millisecs%1000)*1000*1000;
  nanosleep(&s, NULL);
}

unsigned long getMillisec()
{
  ::timeval tp;
  ::gettimeofday(&tp, 0);
  return tp.tv_sec*1000 + tp.tv_usec/1000;
}

void testLoggers(Logger** loggers, const size_t count, const Logger::LogLevel & logLevel)
{
  std::cerr << "***" << std::endl;
  for (int i = 0; i < count; i++) {
    loggers[i]->log_(logLevel, "/// %s, %s\\\\\\", Logger::getLogLevel(logLevel), loggers[i]->getName());
    MutexGuard guard(countersMutex);
    counterAll++;
    if (loggers[i]->isLogLevelEnabled(logLevel))
      counterLogged++;
  }
}

void testLoggers(Logger** loggers, const size_t count, const Logger::LogLevel & logLevel, int num)
{
  std::cerr << "***" << std::endl;
  for (int i = 0; i < count; i++) {
    loggers[i]->log(logLevel, "/// #%6.6u | %s, %s\\\\\\", num, Logger::getLogLevel(logLevel), loggers[i]->getName());
    MutexGuard guard(countersMutex);
    counterAll++;
    if (loggers[i]->isLogLevelEnabled(logLevel))
      counterLogged++;
  }
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
        Logger::getInstance("блах")
    };
    int i=0;
    while (true) {
      {
        MutexGuard guard(mutex);
        if (isStopping)
          return 0;
      }
      std::cerr << "*********************" << std::endl;
      testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_DEBUG, i);
      testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_INFO, i);
      testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_WARN, i);
      testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_ERROR, i);
      testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_FATAL, i);
      testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_NOTSET, i);
      i = (i+1) % 0x8000;
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
    while (true)
    {
      {
        MutexGuard guard(mutex);
        if (isStopping)
          return 0;
      }
      Logger::setLogLevels(levels[i]);
      i = ++i%(int)(sizeof(levels)/sizeof(levels[0]));
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
  Logger::Init();

  std::cout << "*********************" << std::endl;
  Logger::printDebugInfo();

  Logger * loggersForTest[] = {
    Logger::getInstance("map"),
    Logger::getInstance("map.dialog"),
    Logger::getInstance("map.proxy"),
    Logger::getInstance("sms"),
    Logger::getInstance("sms.error"),
    Logger::getInstance("trace"),
    Logger::getInstance("блах")
  };

  std::cout << "*********************" << std::endl;
  Logger::printDebugInfo();

  { /// test loggers levels
    std::cerr << "*********************" << std::endl;
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_DEBUG);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_INFO);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_WARN);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_ERROR);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_FATAL);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_NOTSET);
  }

  { /// print log levels
    std::cout << "*********************" << std::endl;
    std::cout << "Log levels:" << std::endl;
    const Logger::LogLevels * levels = Logger::getLogLevels();
    char *k;
    Logger::LogLevel level;
    for (Logger::LogLevels::Iterator i = levels->getIterator(); i.Next(k, level); )
    {
      std::cout << "  " << k << "->" << Logger::getLogLevel(level) << std::endl;
    }
  }

  { /// test setLogLevels
    Logger::LogLevels levels;
    levels[""] = Logger::LEVEL_INFO;
    levels["map"] = Logger::LEVEL_DEBUG;
    levels["map.proxy"] = Logger::LEVEL_ERROR;

    Logger::setLogLevels(levels);
  }

  std::cout << "*********************" << std::endl;
  Logger::printDebugInfo();

  { /// test loggers levels
    std::cerr << "*********************" << std::endl;
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_DEBUG);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_INFO);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_WARN);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_ERROR);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_FATAL);
    testLoggers(loggersForTest, sizeof(loggersForTest)/sizeof(loggersForTest[0]), Logger::LEVEL_NOTSET);
  }


  TestLogThread1 threads1[256];
//  TestLogThread2 threads2[4];

  counterAll = 0;
  counterLogged = 0;

  unsigned long startTime1 = getMillisec();

  std::cout << "starting..." << std::endl;
  for (int i=0; i<sizeof(threads1)/sizeof(threads1[0]); i++)
  {
    threads1[i].Start();
  }
/*
  for (int i=0; i<sizeof(threads2)/sizeof(threads2[0]); i++)
  {
    threads2[i].Start();
  }
*/

  unsigned long startTime2 = getMillisec();

  std::cout << "started, sleeping..." << std::endl;
  const int secsToSleep = 5*60;
  mySleep(secsToSleep*1000);

  unsigned long stopTime1 = getMillisec();

  std::cout << "stopping..." << std::endl;
  for (int i=0; i<sizeof(threads1)/sizeof(threads1[0]); i++)
  {
    threads1[i].Stop();
    std::cout << "thread1 " << i << " Stop() called" << std::endl;
  }
/*
  for (int i=0; i<sizeof(threads2)/sizeof(threads2[0]); i++)
  {
    threads2[i].Stop();
    std::cout << "thread2 " << i << " Stop() called" << std::endl;
  }
*/

  std::cout << "stopped, waiting..." << std::endl;
  for (int i=0; i<sizeof(threads1)/sizeof(threads1[0]); i++)
  {
    threads1[i].WaitFor();
    std::cout << "thread1 " << i << " stopped" << std::endl;
  }
/*
  for (int i=0; i<sizeof(threads2)/sizeof(threads2[0]); i++)
  {
    threads2[i].WaitFor();
    std::cout << "thread2 " << i << " stopped" << std::endl;
  }
*/

  unsigned long stopTime2 = getMillisec();

  unsigned long startTime = (startTime1 + startTime2) / 2;
  unsigned long stopTime = (stopTime1 + stopTime2) / 2;
  unsigned long runTime = stopTime - startTime;

  std::cout << "threads finished" << std::endl;
  std::cout << "running time: " << ((double)runTime)/1000 << " secs" << std::endl;
  std::cout << "tryes to log: " << counterAll << ", logged: " << counterLogged << std::endl;
  std::cout << "tryes to log: " << ((double)counterAll)*1000/((double)runTime) << " per sec, logged: " << ((double)counterLogged)*1000/((double)runTime) << " per sec" << std::endl;

  std::cout << "shutdown" << std::endl;
  Logger::Shutdown();
  std::cout << "program done" << std::endl;

#else
  std::cout << "Recompile with DEBUG=YES please" << std::endl;
#endif //SMSC_DEBUG
}
