//------------------------------------
//  TimeoutMonitor.cpp
//  Changed by Routman Michael, 2005-2006
//------------------------------------

#include "TimeoutMonitor.hpp"

//#include "Tasks.h"
#include "MCIModule.h"
#include <map>


namespace smsc { namespace mcisme
{

using namespace smsc::misscall;

using namespace smsc::core::synchronization;
using namespace smsc::core::threads;
//using namespace smsc::db;
using std::multimap;
using smsc::logger::Logger;

TimeoutMonitor::TimeoutMonitor(TaskProcessor* _processor, uint32_t to):
	processor(_processor), timeout(to),
	logger(Logger::getInstance("mci.TimeoutM")), count(0) 
	{}

int TimeoutMonitor::Execute()
{
	clearSignalMask();

	int seqNum;	bool timeout=false;
	multimap<time_t, int>::iterator	It;

	while (!bNeedExit)
	{
		{
			MutexGuard lock(awakeMonitor);

			It=seqNums.begin();
			time_t curTime = time(0);
			time_t awakeTime = (It != seqNums.end())?It->first:curTime+600;
			if(awakeTime > curTime)
			{
				timeout = false;
				uint32_t toSleep = (awakeTime - curTime)*1000;
				smsc_log_debug(logger, "TimeoutMonitor: Start wait %d ms", toSleep);
				awakeMonitor.wait(toSleep);
				smsc_log_debug(logger, "TimeoutMonitor: End wait");
			}
			else
			{
				timeout = true;
				smsc_log_debug(logger, "TimeoutMonitor: Timeout has passed for SMS seqNum %d", seqNum);
				seqNum = It->second;
				seqNums.erase(It);
				count--;
			}
		}
		if(timeout) processor->invokeProcessDataSmTimeout(seqNum);
	}
	smsc_log_info(logger, "TimeoutMonitor Exiting ...");
//		exitedEvent.Signal();
	return 0;	
}
void TimeoutMonitor::Start()
{
	MutexGuard guard(startLock);
	
	if (!bStarted)
	{
		smsc_log_info(logger, "TimeoutMonitor Starting ...");
		bNeedExit = false;
		Thread::Start();
		bStarted = true;
		smsc_log_info(logger, "TimeoutMonitor Started.");
	}	
}

void TimeoutMonitor::Stop()
{
	MutexGuard  guard(startLock);
	
	if (bStarted)
	{
		smsc_log_info(logger, "TimeoutMonitor Stopping ...");
		bNeedExit = true;
		awakeMonitor.notify();
//			exitedEvent.Wait();
		WaitFor();
		bStarted = false;
		smsc_log_info(logger, "TimeoutMonitor Stopped.");
	}
}

void TimeoutMonitor::addSeqNum(int seqNum)
{
	MutexGuard lock(awakeMonitor);
	seqNums.insert(multimap<time_t, int>::value_type(time(0) + timeout, seqNum));
	awakeMonitor.notify();
	count++;
	smsc_log_debug(logger, "Added SMS (seqNum = %d) to monitoring list (total = %d).", seqNum, count);
}

void TimeoutMonitor::removeSeqNum(int seqNum)
{
	MutexGuard lock(awakeMonitor);
	multimap<time_t, int>::iterator	It;
	for(It = seqNums.begin(); It != seqNums.end(); ++It)
	{
		if(It->second == seqNum)
		{
			seqNums.erase(It);
			count--;
			awakeMonitor.notify();
			smsc_log_debug(logger, "Removed SMS (seqNum = %d) from monitoring list (total = %d).", seqNum, count);
			return;
		}
	}
	smsc_log_debug(logger, "Removing SMS (seqNum = %d) from monitoring list - failed. No such seqNum. (total = %d)", seqNum, count);
}


};
};
