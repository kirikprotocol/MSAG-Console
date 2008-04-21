//------------------------------------
//  TaskProcessor.hpp
//  Changed by Routman Michael, 2005-2006
//------------------------------------



#ifndef SMSC_MCISME___TASKPROCESSOR_H
#define SMSC_MCISME___TASKPROCESSOR_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <string>

#include <logger/Logger.h>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/CyclicQueue.hpp>

#include <core/threads/Thread.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <system/smscsignalhandlers.h>

//#include "Tasks.h"
#include "MCIModule.h"
#include "MCISmeAdmin.h"
#include "TemplateManager.h"
#include "StatisticsManager.h"
#include "Messages.h"
#include "Storage.hpp"
#include "DeliveryQueue.hpp"
#include "AbntAddr.hpp"
#include "ProfilesStorage.hpp"
#include "TimeoutMonitor.hpp"
#include "advert/Advertising.h"

namespace smsc {
namespace mcisme {

using namespace smsc::misscall;

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::threads;
//using namespace smsc::db;

using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
using scag::advert::Advertising;
using namespace std;

struct MessageSender
{
  virtual int  getSequenceNumber() = 0;
  virtual bool send(int seqNumber, const Message& message) = 0;
  virtual ~MessageSender() {};

protected:

  MessageSender() {};
};

struct sms_info
{
  time_t				sending_time;
  AbntAddr			abnt;
  vector<MCEvent>		events;
};

class TimeoutMonitor;

class TaskProcessor : public Thread, public MissedCallListener, public AdminInterface
{
  std::string test_number;

  smsc::logger::Logger *logger;

  int		protocolId, daysValid;
  std::string	svcType, address;

  int     	releaseCallsStrategy;
  int		stkTemplateId;

  Storage*	pStorage;
  DeliveryQueue*	pDeliveryQueue;

  TemplateManager*	templateManager;
  MCIModule*		mciModule;

  Mutex		messageSenderLock;
  MessageSender*	messageSender;

  Advertising*	advertising;
  bool		useAdvert;

  ProfilesStorage*	profileStorage;
  StatisticsManager*	statistics;
  TimeoutMonitor*		timeoutMonitor;
  time_t			responseWaitTime;

  //EventMonitor    smscIdMonitor;
  Hash<bool>      lockedSmscIds;

  IntHash<sms_info*>	smsInfo;
  Mutex			smsInfoMutex;

  bool	forceInform, forceNotify;

  //EventMonitor		tasksMonitor;

  Mutex   startLock;
  Event   exitedEvent;
  bool    bStarted, bInQueueOpen, bOutQueueOpen, bStopProcessing;
  int                             maxInQueueSize, maxOutQueueSize;
  EventMonitor                    inQueueMonitor, outQueueMonitor;
  CyclicQueue<MissedCallEvent>    inQueue;
  CyclicQueue<Message>            outQueue;

  bool   _groupSmsByCallingAbonent, _isReverseNotifyPolicy;

  void openInQueue();
  void closeInQueue();
  bool putToInQueue(const MissedCallEvent& event, bool skip=true);
  bool getFromInQueue(MissedCallEvent& event);
  string getBanner(const AbntAddr& abnt);
  void test_stk(void);    
  void test_sched(void);
  void test_advert(void);
  //void openOutQueue();
  //void closeOutQueue();
  //bool putToOutQueue(const Message& event, bool force=false);
  //bool getFromOutQueue(Message& event);

  void ProcessAbntEvents(const AbntAddr& abnt);
  bool GetAbntEvents(const AbntAddr& abnt, vector<MCEvent>& events);
  void SendAbntOnlineNotifications(const sms_info* pInfo);
  void StopProcessEvent4Abnt(const AbntAddr& abnt);

  bool noNeedNotify(const AbonentProfile& profile, const sms_info* pInfo) const;
public:

  TaskProcessor(ConfigView* config);
  virtual ~TaskProcessor();

  int getDaysValid()       { return daysValid;  };
  int getProtocolId()      { return protocolId; };

  bool isGroupSmsByCallingAbonent() const { return _groupSmsByCallingAbonent; }

  const char* getSvcType() { return (svcType.c_str()!="") ? svcType.c_str():"MCISme"; };
  const char* getAddress() { return address.c_str(); };

  void assignMessageSender(MessageSender* sender) {
    MutexGuard guard(messageSenderLock);
    messageSender = sender;
  };
  bool isMessageSenderAssigned() {
    MutexGuard guard(messageSenderLock);
    return (messageSender != 0);
  };

  //    void lockSmscId(const char* smsc_id);
  //    void freeSmscId(const char* smsc_id);

  void Run();             // outQueue processing
  virtual int Execute();  // inQueue processing
  void Start(); void Stop();
  void Pause(void);

  virtual void missed(MissedCallEvent event) {
    putToInQueue(event);
  };

  virtual bool invokeProcessDataSmResp(int cmdId, int status, int seqNum);
  //	virtual void invokeProcessDataSmTimeout(int seqNum);
  virtual void invokeProcessDataSmTimeout(void);
  virtual bool invokeProcessAlertNotification(int cmdId, int status, const AbntAddr& abnt);

  /* ------------------------ Admin interface ------------------------ */

  virtual void flushStatistics() {
    if (statistics) statistics->flushStatistics();
  }
  virtual EventsStat getStatistics() {
    return (statistics) ? statistics->getStatistics():EventsStat(0,0,0,0);
  }

  virtual int getActiveTasksCount() {
    if(pDeliveryQueue)
      return pDeliveryQueue->GetAbntCount();
    return 0;
  }
  virtual int getInQueueSize() {
    MutexGuard guard(inQueueMonitor);
    return inQueue.Count();
  }
  virtual int getOutQueueSize() {
    if(pDeliveryQueue)
      return pDeliveryQueue->GetQueueSize();
    return 0;
  }
  virtual string getSchedItem(const string& Abonent);
  virtual string getSchedItems(void);
};

//class TimeoutMonitor : public Thread
//{
//	smsc::logger::Logger*   logger;
//	TaskProcessor*		processor;
//	uint32_t			timeout;
//	map<time_t, int>	seqNums;
//	EventMonitor		awakeMonitor;
//	Mutex				startLock;
//	Event				exitedEvent;
//	bool				bStarted, bNeedExit;
//
//public:
//	
//	TimeoutMonitor(TaskProcessor* _processor, uint32_t to):
//	  processor(_processor), timeout(to),
//	  logger(Logger::getInstance("mci.TimeoutM")) 
//	  {}
//    virtual int Execute()
//	{
//		clearSignalMask();
//
//		while (!bNeedExit)
//		{
//			MutexGuard lock(awakeMonitor);
//			map<time_t, int>::iterator	It=seqNums.begin();
//			
//			time_t curTime = time(0);
//			time_t awakeTime = (It != seqNums.end())?It->first:curTime+600;
//			if(awakeTime > curTime)
//			{
//				uint32_t toSleep = (awakeTime - curTime)*1000;
//				smsc_log_debug(logger, "TimeoutMonitor: Start wait %d ms", toSleep);
//				awakeMonitor.wait(toSleep);
//				smsc_log_debug(logger, "TimeoutMonitor: End wait");
//			}
//			else
//			{
//				int seqNum = It->second;
//				smsc_log_debug(logger, "TimeoutMonitor: Timeout has passed for SMS seqNum %d", seqNum);
//				processor->invokeProcessDataSmTimeout(seqNum);
//				seqNums.erase(It);
//			}
//		}
//		smsc_log_info(logger, "TimeoutMonitor Exiting ...");
////		exitedEvent.Signal();
//		return 0;	
//	}
//	void Start()
//	{
//		MutexGuard guard(startLock);
//	    
//		if (!bStarted)
//		{
//			smsc_log_info(logger, "TimeoutMonitor Starting ...");
//			bNeedExit = false;
//			Thread::Start();
//			bStarted = true;
//			smsc_log_info(logger, "TimeoutMonitor Started.");
//		}	
//	}
//	void Stop()
//	{
//		MutexGuard  guard(startLock);
//	    
//		if (bStarted)
//		{
//			smsc_log_info(logger, "TimeoutMonitor Stopping ...");
//			bNeedExit = true;
//			awakeMonitor.notify();
////			exitedEvent.Wait();
//			WaitFor();
//			bStarted = false;
//			smsc_log_info(logger, "TimeoutMonitor Stopped.");
//		}
//	}
//
//	void addSeqNum(int seqNum)
//	{
//		MutexGuard lock(awakeMonitor);
//		seqNums.insert(multimap<time_t, int>::value_type(time(0) + timeout, seqNum));
//		awakeMonitor.notify();
//		smsc_log_debug(logger, "Added SMS (seqNum = %d) to monitoring list (total = %d).", seqNum, seqNums.size());
//	}
//	void removeSeqNum(int seqNum)
//	{
//		MutexGuard lock(awakeMonitor);
//		map<time_t, int>::iterator	It;
//		for(It = seqNums.begin(); It != seqNums.end(); ++It)
//		{
//			if(It->second == seqNum)
//			{
//				seqNums.erase(It);
//				awakeMonitor.notify();
//				smsc_log_debug(logger, "Removed SMS (seqNum = %d) from monitoring list (total = %d).", seqNum, seqNums.size());
//				return;
//			}
//		}
//		smsc_log_debug(logger, "Removing SMS (seqNum = %d) from monitoring list - failed. No such seqNum. (total = %d)", seqNum, seqNums.size());
//	}
//	
//};


};
};

#endif
