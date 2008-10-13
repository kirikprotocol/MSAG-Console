#ifndef __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__
# define __SMSC_MCISME_OUTPUTMESSAGEPROCESSOR_HPP__

# include <logger/Logger.h>
# include <util/config/ConfigView.h>
# include <core/threads/Thread.hpp>
# include <core/synchronization/EventMonitor.hpp>

# include <mcisme/AbntAddr.hpp>
# include <mcisme/advert/Advertising.h>
# include <mcisme/TaskProcessor.h>
# include <mcisme/OutputMessageProcessorsDispatcher.hpp>

namespace smsc {
namespace mcisme {

class OutputMessageProcessor : public smsc::core::threads::Thread {
public:
  OutputMessageProcessor(TaskProcessor& taskProcessor, util::config::ConfigView* advertCfg, OutputMessageProcessorsDispatcher& dispatcher);
  virtual int Execute();

  void assignMessageOutputWork(const AbntAddr& calledAbnt);
  std::string getBanner(const AbntAddr& abnt);

  void stop();
private:
  bool waitForCalledAbonentInfo(AbntAddr* calledAbnt);
  void formOutputMessageAndSendIt(const AbntAddr& abnt);

  TaskProcessor& _taskProcessor;
  AbntAddr _calledAbnt;
  bool _isStopped, _eventWasSignalled;
  smsc::core::synchronization::EventMonitor _abonentInfoEventMonitor;
  Advertising* _advertising;
  logger::Logger* _logger;
  int _connectTimeout;
  OutputMessageProcessorsDispatcher& _messagesProcessorsDispatcher;
};

}}

#endif
