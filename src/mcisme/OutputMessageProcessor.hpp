#ifndef __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__
# define __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__

# include "logger/Logger.h"
# include "util/config/ConfigView.h"
# include "core/threads/Thread.hpp"
# include "core/synchronization/EventMonitor.hpp"
# include "mcisme/Profiler.h"
# include "mcisme/AbntAddr.hpp"
# include "mcisme/TaskProcessor.h"
# include "mcisme/advert/AdvertisingImpl.h"
# include "mcisme/advert/BEReconnector.hpp"
# include "mcisme/OutputMessageProcessorsDispatcher.hpp"

namespace smsc {
namespace mcisme {

struct sms_info;
class BannerReader;

class SendMessageEventHandler {
public:
  SendMessageEventHandler(TaskProcessor& taskProcessor, BEReconnector& reconnectorThread,
                          AdvertImplRefPtr& advertising)
    : _taskProcessor(taskProcessor), _reconnectorThread(reconnectorThread),
      _advertising(advertising), _logger(logger::Logger::getInstance("outputprc")) {}
  virtual ~SendMessageEventHandler() {}
  virtual void handle() = 0;
  // return false if banner request was sent successfully, else return true.
  bool sendBannerRequest(const AbntAddr& abnt,
                         bool need_banner_in_translit,
                         MCEventOut* mc_event_out,
                         uint32_t max_banner_size);
  void rollbackBanner(const BannerResponseTrace& bannerRespTrace);
protected:
  TaskProcessor& _taskProcessor;
  BEReconnector& _reconnectorThread;
  AdvertImplRefPtr _advertising;
  logger::Logger* _logger;
};

class SendAbonentOnlineNotificationEventHandler : public SendMessageEventHandler {
public:
  SendAbonentOnlineNotificationEventHandler(TaskProcessor& taskProcessor,
                                            BEReconnector& reconnectorThread,
                                            AdvertImplRefPtr& advertising,
                                            const sms_info* pInfo,
                                            const AbonentProfile& abntProfile)
    : SendMessageEventHandler(taskProcessor, reconnectorThread, advertising),
      _pInfo(pInfo), _abntProfile(abntProfile) {}
  virtual ~SendAbonentOnlineNotificationEventHandler() { delete _pInfo; }
  virtual void handle();
private:
  const sms_info* _pInfo;
  AbonentProfile _abntProfile;
};

class SendMissedCallMessageEventHandler : public SendMessageEventHandler {
public:
  SendMissedCallMessageEventHandler(TaskProcessor& taskProcessor,
                                    BEReconnector& reconnectorThread,
                                    AdvertImplRefPtr& advertising,
                                    const AbntAddr& calledAbnt)
    : SendMessageEventHandler(taskProcessor, reconnectorThread, advertising), _calledAbnt(calledAbnt) {}
  virtual void handle();
private:
  void formOutputMessageAndSendIt(const AbntAddr& abnt);

  AbntAddr _calledAbnt;
};

class RollbackBERequestHandler : public SendMessageEventHandler {
public:
  RollbackBERequestHandler(TaskProcessor& taskProcessor,
                           BEReconnector& reconnectorThread,
                           AdvertImplRefPtr& advertising,
                           const BannerResponseTrace& bannerRespTrace)
    : SendMessageEventHandler(taskProcessor, reconnectorThread, advertising),
      _bannerRespTrace(bannerRespTrace)
  {}
  virtual void handle();
private:
  BannerResponseTrace _bannerRespTrace;
};

class OutputMessageProcessor : public core::threads::Thread {
public:
  OutputMessageProcessor(TaskProcessor& task_processor,
                         util::config::ConfigView* advert_cfg,
                         OutputMessageProcessorsDispatcher& dispatcher,
                         BEReconnector& reconnector_thread,
                         BannerReader& banner_reader);
  virtual int Execute();

  void assignMessageOutputWork(const AbntAddr& calledAbnt);
  void assignMessageOutputWork(const sms_info* pInfo, const AbonentProfile& abntProfile);
  void assignMessageOutputWork(const BannerResponseTrace& bannerRespTrace);

  void stop();
private:
  SendMessageEventHandler* waitingForHandler();

  TaskProcessor& _taskProcessor;

  bool _isStopped, _eventWasSignalled;
  core::synchronization::EventMonitor _outputEventMonitor;
  AdvertImplRefPtr _advertising;
  logger::Logger* _logger;
  SendMessageEventHandler* _handler;
  OutputMessageProcessorsDispatcher& _messagesProcessorsDispatcher;
  BEReconnector& _reconnectorThread;
};

}}

#endif
