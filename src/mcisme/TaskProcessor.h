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

#include "logger/Logger.h"

#include "util/Exception.hpp"
#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"

#include "core/buffers/Array.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/CyclicQueue.hpp"

#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "system/smscsignalhandlers.h"

#include "mcisme/OutputMessageProcessorsDispatcher.hpp"

#include "mcisme/MCIModule.h"
#include "mcisme/MCISmeAdmin.h"
#include "mcisme/TemplateManager.h"
#include "mcisme/StatisticsManager.h"
#include "mcisme/Messages.h"
#include "mcisme/Storage.hpp"
#include "mcisme/DeliveryQueue.hpp"
#include "mcisme/AbntAddr.hpp"
#include "mcisme/ProfilesStorage.hpp"
#include "mcisme/TimeoutMonitor.hpp"
#include "mcisme/IASMEProxy.hpp"
#include "mcisme/advert/Advertising.h"
#include "mcisme/IAProtocolHandler.hpp"

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

struct sms_info;
struct TimeoutItem{
  int seqNum;
  sms_info* info;
  time_t expirationTime;
  TimeoutItem(int argSeqNum,time_t argExpirationTime,sms_info* argInfo):seqNum(argSeqNum),expirationTime(argExpirationTime),info(argInfo)
  {
  }
};

typedef std::list<TimeoutItem> TimeoutList;

struct sms_info
{
  time_t          sending_time;
  AbntAddr        abnt;
  vector<MCEvent> events;
  BannerResponseTrace bannerRespTrace;
  TimeoutList::iterator timeoutIter;
};

class SendMessageEventHandler;
class TimeoutMonitor;

class RetryException : public util::Exception {
public:
  RetryException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class TaskProcessor : public Thread, public MissedCallListener,
                      public AdminInterface, public IAProtocolHandler
{
  std::string test_number;

  smsc::logger::Logger *logger;

  int          protocolId, daysValid;
  std::string  svcType, svcTypeOnLine, svcTypeForBe, address;

  uint32_t _qosTimeToLive; //in minutes
  int      releaseCallsStrategy;
  int      stkTemplateId;

  Storage*         pStorage;
  DeliveryQueue*   pDeliveryQueue;

  TemplateManager* templateManager;
  MCIModule*       mciModule;

  mutable Mutex    messageSenderLock;
  MessageSender* messageSender;

  Advertising*   advertising;

  ProfilesStorage*   profileStorage;
  StatisticsManager* statistics;
  TimeoutMonitor*    timeoutMonitor;
  time_t             responseWaitTime;

  Hash<bool>      lockedSmscIds;

  TimeoutList toList;
  IntHash<sms_info*> smsInfo;
  mutable Mutex	     smsInfoMutex;

  IntHash<BannerResponseTrace> _bannerInNotificationRegistry;
  mutable Mutex _bannerInNotificationRegistryLock;

  void insertBannerInfo(int seqNum, const BannerResponseTrace& bannerRespTrace) {
    core::synchronization::MutexGuard synchronize(_bannerInNotificationRegistryLock);
    _bannerInNotificationRegistry.Insert(seqNum, bannerRespTrace);
  }

  BannerResponseTrace deleteBannerInfo(int seqNum) {
    core::synchronization::MutexGuard synchronize(_bannerInNotificationRegistryLock);
    BannerResponseTrace bannerRespTrace;
    if ( _bannerInNotificationRegistry.Exist(seqNum) ) {
      bannerRespTrace = _bannerInNotificationRegistry.Get(seqNum);
      _bannerInNotificationRegistry.Delete(seqNum);
    }
    return bannerRespTrace;
  }

  bool    forceInform, forceNotify;

  Mutex   startLock;
  Event   exitedEvent;
  bool    bStarted, bInQueueOpen, bOutQueueOpen, bStopProcessing;
  int maxInQueueSize;
  mutable EventMonitor inQueueMonitor;
  CyclicQueue<MissedCallEvent> inQueue;

  bool _isUseWantNotifyPolicy;
  bool _originatingAddressIsMCIAddress;
  time_t _sendAbntOnlineNotificationPeriod; // expressed in seconds

  size_t _maxMessageSize;

  int maxDataSmRegistrySize;

  OutputMessageProcessorsDispatcher* _outputMessageProcessorsDispatcher;
  IASMEProxy* _iasmeProxy;

  void openInQueue();
  void closeInQueue();
  bool putToInQueue(const MissedCallEvent& event, bool skip=true);
  bool getFromInQueue(MissedCallEvent& event);

  bool GetAbntEvents(const AbntAddr& abnt, vector<MCEvent>& events);

  bool needNotify(const AbonentProfile& profile, const sms_info* pInfo) const;

  void store_D_Event_in_logstore(const AbntAddr& abnt,
                                 const vector<MCEvent>& events,
                                 const AbonentProfile& abntProfile);

  void store_F_Event_in_logstore(const AbntAddr& abnt,
                                 const vector<MCEvent>& events);

  void store_N_Event_in_logstore(const std::string& calledAbonent,
                                 const std::string& callingAbonent);

  void store_A_Event_in_logstore(const AbntAddr& callingAbonent,
                                 const AbntAddr& calledAbonent,
                                 const AbonentProfile& abntProfile,
				 const AbonentProfile& callerProfile);

  MessageSender* getMessageSender() {
    MutexGuard guard(messageSenderLock);
    if ( !messageSender )
      throw RetryException("TaskProcessor::getMessageSender::: messageSender is not set");
    return messageSender;
  }

  void insertSmsInfo(int seqNum, sms_info* pInfo) {
    MutexGuard Lock(smsInfoMutex);
    pInfo->timeoutIter=toList.insert(toList.end(),TimeoutItem(seqNum,pInfo->sending_time+responseWaitTime,pInfo));
    smsInfo.Insert(seqNum, pInfo);
  }

  void deleteSmsInfo(int seqNum) {
    MutexGuard Lock(smsInfoMutex);
    sms_info* ptr=smsInfo.Get(seqNum);
    toList.erase(ptr->timeoutIter);
    smsInfo.Delete(seqNum);
  }
  void processLocally(const AbntAddr& from, const AbntAddr& to,
                      const MissedCallEvent& event,
                      const AbonentProfile& profile);
  bool forwardEventToIASME(const MissedCallEvent &event);

public:

  TaskProcessor(ConfigView* config);
  virtual ~TaskProcessor();

  void ProcessAbntEvents(const AbntAddr& abnt, SendMessageEventHandler* bannerEngineProxy=NULL);
  void SendAbntOnlineNotifications(const sms_info* pInfo,
                                   const AbonentProfile& profile,
                                   SendMessageEventHandler* bannerEngineProxy=NULL);
  int processNotificationMessage(const Message& msg);
  void processNotificationMessage(const BannerResponseTrace& banner_resp_trace,
                                  const Message& msg);

  bool sendMessage(const MCEventOut& outEvent,
                   const BannerResponseTrace& bannerRespTrace);

  int getDaysValid() const { return daysValid;  }
  int getProtocolId() const { return protocolId; }

  uint32_t getQosTimeToLive() const { return _qosTimeToLive; }

  const char* getSvcType() const { return (svcType != "") ? svcType.c_str():"MCISme"; }
  const char* getSvcTypeOnLine() const { return (svcTypeOnLine != "") ? svcTypeOnLine.c_str():"MCISme"; }
  const char* getSvcTypeForBE() const { return (svcTypeForBe != "") ? svcTypeForBe.c_str(): getSvcType(); }

  const std::string& getAddress() const { return address; }

  void assignMessageSender(MessageSender* sender) {
    MutexGuard guard(messageSenderLock);
    messageSender = sender;
  }
  bool isMessageSenderAssigned() const {
    MutexGuard guard(messageSenderLock);
    return (messageSender != 0);
  }

  void Run();             // outQueue processing
  virtual int Execute();  // inQueue processing

  using Thread::Start;

  void Start(); void Stop();
  void Pause(void);

  virtual void missed(const MissedCallEvent& event) {
    putToInQueue(event);
  }

  bool invokeProcessDataSmResp(int cmdId, int status, int seqNum);
  bool invokeProcessSubmitSmResp(int cmdId, int status, int seqNum);
  void invokeProcessDataSmTimeout(void);
  bool invokeProcessAlertNotification(int cmdId, int status, const AbntAddr& abnt);

  void commitMissedCallEvents(const AbntAddr& abonent,
                              const std::vector<MCEvent>& src_events,
                              const AbonentProfile& abntProfile);
  /* ------------------------ Admin interface ------------------------ */

  virtual void flushStatistics() {
    if (statistics) statistics->flushStatistics();
  }
  virtual EventsStat getStatistics() const {
    return (statistics) ? statistics->getStatistics():EventsStat(0,0,0,0);
  }

  virtual unsigned getActiveTasksCount() const {
    if(pDeliveryQueue)
      return pDeliveryQueue->GetAbntCount();
    return 0;
  }
  virtual unsigned getInQueueSize() const {
    MutexGuard guard(inQueueMonitor);
    return inQueue.Count();
  }
  virtual unsigned getOutQueueSize() const {
    if(pDeliveryQueue)
      return pDeliveryQueue->GetQueueSize();
    return 0;
  }
  virtual string getSchedItem(const string& Abonent);
  virtual string getSchedItems(void);

  virtual void handle(const mcaia::BusyResponse& msg);
};

}
}

#endif
