#ifndef __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__
# define __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__

# include <logger/Logger.h>
# include <util/config/ConfigView.h>
# include <core/threads/Thread.hpp>
# include <core/synchronization/EventMonitor.hpp>

# include <mcisme/Profiler.h>
# include <mcisme/AbntAddr.hpp>
# include <mcisme/advert/Advertising.h>
# include <mcisme/TaskProcessor.h>
# include <mcisme/OutputMessageProcessorsDispatcher.hpp>

namespace smsc {
namespace mcisme {

struct sms_info;

class SendMessageEventHandler {
public:
  SendMessageEventHandler(TaskProcessor& taskProcessor, int connectTimeout, Advertising* advertising)
    : _taskProcessor(taskProcessor), _connectTimeout(connectTimeout),
      _advertising(advertising), _logger(logger::Logger::getInstance("outputprc")) {}
  virtual ~SendMessageEventHandler() {}
  virtual void handle() = 0;
  std::string getBanner(const AbntAddr& abnt);
protected:
  TaskProcessor& _taskProcessor;
  int _connectTimeout;
  Advertising* _advertising;
  logger::Logger* _logger;
};

class SendAbonentOnlineNotificationEventHandler : public SendMessageEventHandler {
public:
  SendAbonentOnlineNotificationEventHandler(TaskProcessor& taskProcessor,
                                            int connectTimeout,
                                            Advertising* advertising,
                                            const sms_info* pInfo,
                                            const AbonentProfile& abntProfile)
    : SendMessageEventHandler(taskProcessor, connectTimeout, advertising),
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
                                    int connectTimeout,
                                    Advertising* advertising,
                                    const AbntAddr& calledAbnt)
    : SendMessageEventHandler(taskProcessor, connectTimeout, advertising), _calledAbnt(calledAbnt) {}
  virtual void handle();
private:
  void formOutputMessageAndSendIt(const AbntAddr& abnt);

  AbntAddr _calledAbnt;
};

class OutputMessageProcessor : public smsc::core::threads::Thread {
public:
  OutputMessageProcessor(TaskProcessor& taskProcessor,
                         util::config::ConfigView* advertCfg,
                         OutputMessageProcessorsDispatcher& dispatcher);
  virtual int Execute();

  void assignMessageOutputWork(const AbntAddr& calledAbnt);
  void assignMessageOutputWork(const sms_info* pInfo, const AbonentProfile& abntProfile);

  void stop();
private:
  SendMessageEventHandler* waitingForHandler();

  TaskProcessor& _taskProcessor;

  bool _isStopped, _eventWasSignalled;
  smsc::core::synchronization::EventMonitor _outputEventMonitor;
  Advertising* _advertising;
  logger::Logger* _logger;
  SendMessageEventHandler* _handler;
  int _connectTimeout;
  OutputMessageProcessorsDispatcher& _messagesProcessorsDispatcher;
};

}}

#endif
