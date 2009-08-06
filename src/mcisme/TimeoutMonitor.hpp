//------------------------------------
//  TimeoutMonitor.hpp
//  Changed by Routman Michael, 2005-2006
//------------------------------------

#ifndef SMSC_MCISME___TIMEOUTMONITOR_H
#define SMSC_MCISME___TIMEOUTMONITOR_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <map>

#include <logger/Logger.h>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/threads/Thread.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <system/smscsignalhandlers.h>
#include "TaskProcessor.h"

//#include "Tasks.h"
#include "MCIModule.h"

namespace smsc {
namespace mcisme {

using namespace smsc::misscall;

using namespace smsc::core::synchronization;
using namespace smsc::core::threads;
//using namespace smsc::db;
using std::multimap;
using smsc::logger::Logger;
class TaskProcessor;

class TimeoutMonitor : public Thread
{
  smsc::logger::Logger*   logger;
  TaskProcessor*			processor;

  Event		awakeEvent;
  Mutex		startLock;
  bool		bStarted, bNeedExit;

public:

  TimeoutMonitor(TaskProcessor* _processor);
  virtual int Execute();

  using Thread::Start;
  void Start();
  void Stop();
};


//class TimeoutMonitor : public Thread
//{
//	smsc::logger::Logger*   logger;
//	TaskProcessor*			processor;
//	uint32_t				timeout;
//	multimap<time_t, int>	seqNums;
//	EventMonitor			awakeMonitor;
//	Mutex					startLock;
//	Event					exitedEvent;
//	bool					bStarted, bNeedExit;
//	uint32_t				count;
//
//public:
//	
//	TimeoutMonitor(TaskProcessor* _processor, uint32_t to);
//    virtual int Execute();
//	void Start();
//	void Stop();
//	void addSeqNum(int seqNum);
//	void removeSeqNum(int seqNum);
//};

};
};

#endif
