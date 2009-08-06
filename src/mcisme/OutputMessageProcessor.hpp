#ifndef __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__
# define __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__

# include <logger/Logger.h>
# include <util/config/ConfigView.h>
# include <core/threads/Thread.hpp>
# include <core/synchronization/EventMonitor.hpp>
# include <core/buffers/RefPtr.hpp>
# include <mcisme/Profiler.h>
# include <mcisme/AbntAddr.hpp>
# include <mcisme/TaskProcessor.h>
# include <mcisme/advert/AdvertisingImpl.h>
# include <mcisme/advert/BEReconnector.hpp>
# include <mcisme/OutputMessageProcessorsDispatcher.hpp>

namespace smsc {
namespace mcisme {

struct sms_info;

class SendMessageEventHandler {
public:
  SendMessageEventHandler(TaskProcessor& taskProcessor, BEReconnector& reconnectorThread,
                          core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& advertising)
    : _taskProcessor(taskProcessor), _reconnectorThread(reconnectorThread),
      _advertising(advertising), _logger(logger::Logger::getInstance("outputprc")) {}
  virtual ~SendMessageEventHandler() {}
  virtual void handle() = 0;
  std::string getBanner(const AbntAddr& abnt,
                        BannerResponseTrace* banner_resp_trace,
                        bool need_banner_in_translit,
                        uint32_t max_banner_size);
  void rollbackBanner(const BannerResponseTrace& bannerRespTrace);
protected:
  TaskProcessor& _taskProcessor;
  BEReconnector& _reconnectorThread;
  core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex> _advertising;
  logger::Logger* _logger;
};

class SendAbonentOnlineNotificationEventHandler : public SendMessageEventHandler {
public:
  SendAbonentOnlineNotificationEventHandler(TaskProcessor& taskProcessor,
                                            BEReconnector& reconnectorThread,
                                            core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& advertising,
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
                                    core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& advertising,
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
                           core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& advertising,
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
  OutputMessageProcessor(TaskProcessor& taskProcessor,
                         util::config::ConfigView* advertCfg,
                         OutputMessageProcessorsDispatcher& dispatcher,
                         BEReconnector& reconnectorThread);
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
  core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex> _advertising;
  logger::Logger* _logger;
  SendMessageEventHandler* _handler;
  OutputMessageProcessorsDispatcher& _messagesProcessorsDispatcher;
  BEReconnector& _reconnectorThread;
};

}}

#endif
