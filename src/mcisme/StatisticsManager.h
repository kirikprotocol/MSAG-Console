#ifndef SMSC_MCI_SME_STATISTICS_MANAGER
#define SMSC_MCI_SME_STATISTICS_MANAGER

#include "Statistics.h"
#include <core/buffers/File.hpp>

#include <logger/Logger.h>

#include <core/threads/Thread.hpp>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

namespace smsc {
namespace mcisme {

using namespace core::threads;
using namespace core::synchronization;
using smsc::core::buffers::File;
    
using smsc::logger::Logger;
    
class StatisticsManager : public Statistics, public Thread
{
protected:
    
  smsc::logger::Logger*   logger;
  smsc::logger::Logger*   processLog;
        
  EventsStat              statistics[2];

  short   currentIndex;
  bool    bExternalFlush;
        
  Mutex   startLock, switchLock, flushLock;
        
  Event   awakeEvent, exitEvent, doneEvent;
  bool    bStarted, bNeedExit;
        
  short switchCounters();
  void  flushCounters(short index);

  //uint32_t calculatePeriod();
  int      calculateToSleep();

  std::string     location;
  bool            bFileTM;
  tm              fileTM;
  File file;

  bool createStorageDir(const std::string loc);
  void dumpCounters(const uint8_t* buff, uint32_t buffLen, const tm& flushTM);
  bool createDir(const std::string& dir);
  void calculateTime(tm& flushTM);

public:
        
  virtual int Execute();
  virtual void Start();
  virtual void Stop();

  virtual void flushStatistics();
  virtual EventsStat getStatistics();

  virtual void incMissed   (const char* abonent);
  virtual void incDelivered(const char* abonent);
  virtual void incFailed   (const char* abonent);
  virtual void incNotified (const char* abonent);
  virtual void incMissed   (unsigned count = 1);
  virtual void incDelivered(unsigned count = 1);
  virtual void incFailed   (unsigned count = 1);
  virtual void incNotified (unsigned count = 1);
        
  StatisticsManager(const std::string& loc);
  virtual ~StatisticsManager();
};

}}

#endif //SMSC_MCI_SME_STATISTICS_MANAGER
