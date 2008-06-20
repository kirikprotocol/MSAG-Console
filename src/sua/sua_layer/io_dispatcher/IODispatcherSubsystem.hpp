#ifndef __SUA_SUALAYER_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__
# define __SUA_SUALAYER_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__ 1

# include <core/threads/ThreadPool.hpp>

# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/ApplicationSubsystem.hpp>
# include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
# include <sua/sua_layer/runtime_cfg/ParameterObserver.hpp>

namespace io_dispatcher {

class IODispatcherSubsystem : public sua_layer::ApplicationSubsystem,
                              public runtime_cfg::ParameterObserver,
                              public utilx::Singleton<IODispatcherSubsystem*> {
public:
  IODispatcherSubsystem();
  virtual void start();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::changeParameterEventHandler;
  virtual void changeParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                           const runtime_cfg::Parameter& modifiedParameter);
private:
  smsc::core::threads::ThreadPool _threadsPool;
  int _stateMachinesCount;
};

}

#endif
