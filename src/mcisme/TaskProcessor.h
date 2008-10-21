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

#include <mcisme/OutputMessageProcessorsDispatcher.hpp>

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

using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

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
  time_t          sending_time;
  AbntAddr        abnt;
  time_t          lastCallingTime;
  vector<MCEvent> events;
};

class SendMessageEventHandler;
class TimeoutMonitor;

class TaskProcessor : public Thread, public MissedCallListener, public AdminInterface
{
  std::string test_number;

  smsc::logger::Logger *logger;

  int              protocolId, daysValid;
  std::string      svcType, svcTypeOnLine, address;

  int              releaseCallsStrategy;
  int              stkTemplateId;

  Storage*         pStorage;
  DeliveryQueue*   pDeliveryQueue;

  TemplateManager* templateManager;
  MCIModule*     mciModule;

  Mutex          messageSenderLock;
  MessageSender* messageSender;

  Advertising*   advertising;

  ProfilesStorage*   profileStorage;
  StatisticsManager* statistics;
  TimeoutMonitor*    timeoutMonitor;
  time_t             responseWaitTime;

  Hash<bool>      lockedSmscIds;

  IntHash<sms_info*> smsInfo;
  Mutex	             smsInfoMutex;

  bool    forceInform, forceNotify;

  Mutex   startLock;
  Event   exitedEvent;
  bool    bStarted, bInQueueOpen, bOutQueueOpen, bStopProcessing;
  int                             maxInQueueSize; //, maxOutQueueSize;
  EventMonitor                    inQueueMonitor; //, outQueueMonitor;
  CyclicQueue<MissedCallEvent>    inQueue;

  bool _isUseWantNotifyPolicy;
  bool _originatingAddressIsMCIAddress;
  time_t _sendAbntOnlineNotificationPeriod; // expressed in seconds

  OutputMessageProcessorsDispatcher* _outputMessageProcessorsDispatcher;

  void openInQueue();
  void closeInQueue();
  bool putToInQueue(const MissedCallEvent& event, bool skip=true);
  bool getFromInQueue(MissedCallEvent& event);

  bool GetAbntEvents(const AbntAddr& abnt, vector<MCEvent>& events);

  bool needNotify(const AbonentProfile& profile, const sms_info* pInfo) const;

  bool sendMessage(const AbntAddr& abnt, const Message& msg, const MCEventOut& outEvent);

  void store_D_Event_in_logstore(const AbntAddr& abnt,
                                 const vector<MCEvent>& events,
                                 const AbonentProfile& abntProfile);

  void store_F_Event_in_logstore(const AbntAddr& abnt,
                                 const vector<MCEvent>& events);

  void store_N_Event_in_logstore(const std::string& calledAbonent,
                                 const std::string& callingAbonent);

  void store_A_Event_in_logstore(const AbntAddr& callingAbonent,
                                 const AbntAddr& calledAbonent,
                                 const AbonentProfile& abntProfile);
public:

  TaskProcessor(ConfigView* config);
  virtual ~TaskProcessor();

  void ProcessAbntEvents(const AbntAddr& abnt, SendMessageEventHandler* bannerEngineProxy=NULL);
  void SendAbntOnlineNotifications(const sms_info* pInfo,
                                   const AbonentProfile& profile,
                                   SendMessageEventHandler* bannerEngineProxy=NULL);

  int getDaysValid()       { return daysValid;  }
  int getProtocolId()      { return protocolId; }

  const char* getSvcType() const { return (svcType.c_str()!="") ? svcType.c_str():"MCISme"; }
  const char* getSvcTypeOnLine() const { return (svcTypeOnLine.c_str()!="") ? svcTypeOnLine.c_str():"MCISme"; }
  const std::string& getAddress() const { return address; }

  void assignMessageSender(MessageSender* sender) {
    MutexGuard guard(messageSenderLock);
    messageSender = sender;
  }
  bool isMessageSenderAssigned() {
    MutexGuard guard(messageSenderLock);
    return (messageSender != 0);
  }

  void Run();             // outQueue processing
  virtual int Execute();  // inQueue processing
  void Start(); void Stop();
  void Pause(void);

  virtual void missed(MissedCallEvent event) {
    putToInQueue(event);
  }

  virtual bool invokeProcessDataSmResp(int cmdId, int status, int seqNum);
  virtual void invokeProcessDataSmTimeout(void);
  virtual bool invokeProcessAlertNotification(int cmdId, int status, const AbntAddr& abnt);

  void commitMissedCallEvents(const sms_info* pInfo, const AbonentProfile& abntProfile);
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

}
}

#endif
