#ifndef __SUA_SUALAYER_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__
# define __SUA_SUALAYER_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__ 1

# include <sua/utilx/Subsystem.hpp>
# include <sua/utilx/Singleton.hpp>
# include <util/config/ConfigView.h>
# include <core/threads/ThreadPool.hpp>
# include <logger/Logger.h>

namespace io_dispatcher {

class IODispatcherSubsystem : public utilx::Subsystem,
                              public utilx::Singleton<IODispatcherSubsystem*> {
public:
  IODispatcherSubsystem();
  virtual void start();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);
  virtual const std::string& getName() const;
  virtual void waitForCompletion();
private:
  std::string _name;

  smsc::core::threads::ThreadPool _threadsPool;
  smsc::logger::Logger* _logger;
  int _stateMachinesCount;
};

}

#endif
