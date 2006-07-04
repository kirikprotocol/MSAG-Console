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

namespace smsc { namespace mcisme
{
using namespace smsc::misscall;

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::threads;
//using namespace smsc::db;

using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

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
	AbntAddr			abnt;
	vector<MCEvent>		events;
};

class TaskProcessor : public Thread, public MissedCallListener, public AdminInterface
{
	std::string test_number;

  smsc::logger::Logger *logger;

    int     protocolId, daysValid;
    char    *svcType, *address;

    int     releaseCallsStrategy;

	Storage*			pStorage;
	DeliveryQueue*		pDeliveryQueue;

	TemplateManager*    templateManager;
    MCIModule*          mciModule;

    Mutex               messageSenderLock;
    MessageSender*      messageSender;
    
	ProfilesStorage*	profileStorage;
    StatisticsManager*  statistics;
    
//    EventMonitor    smscIdMonitor;
    Hash<bool>      lockedSmscIds;

	IntHash<sms_info*>	smsInfo;
	Mutex				smsInfoMutex;
	
	bool	forceInform, forceNotify;

//	EventMonitor		tasksMonitor;

    Mutex   startLock;
    Event   exitedEvent;
    bool    bStarted, bInQueueOpen, bOutQueueOpen;
    int                             maxInQueueSize, maxOutQueueSize;
    EventMonitor                    inQueueMonitor, outQueueMonitor;
    CyclicQueue<MissedCallEvent>    inQueue;
    CyclicQueue<Message>            outQueue;
    
    void openInQueue();
    void closeInQueue();
    bool putToInQueue(const MissedCallEvent& event, bool skip=true);
    bool getFromInQueue(MissedCallEvent& event);
    
    //void openOutQueue();
    //void closeOutQueue();
    //bool putToOutQueue(const Message& event, bool force=false);
    //bool getFromOutQueue(Message& event);
    
	void ProcessAbntEvents(const AbntAddr& abnt);
	bool GetAbntEvents(const AbntAddr& abnt, vector<MCEvent>& events);
	void SendAbntOnlineNotifications(const sms_info* pInfo);
	void StopProcessEvent4Abnt(const AbntAddr& abnt);

public:

    TaskProcessor(ConfigView* config);
    virtual ~TaskProcessor();

    int getDaysValid()       { return daysValid;  };
    int getProtocolId()      { return protocolId; };
    const char* getSvcType() { return (svcType) ? svcType:"MCISme"; };
    const char* getAddress() { return address; };

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
    
    virtual void missed(MissedCallEvent event) {
        putToInQueue(event);
    };
    
    virtual bool invokeProcessDataSmResp(int cmdId, int status, int seqNum);
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
};

};
};

#endif
