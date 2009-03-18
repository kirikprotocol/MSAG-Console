#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__

# include <core/threads/ThreadPool.hpp>

# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
# include <eyeline/utilx/runtime_cfg/ParameterObserver.hpp>
# include <eyeline/sua/sua_layer/ApplicationSubsystem.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

class IODispatcherSubsystem : public sua_layer::ApplicationSubsystem,
                              public utilx::runtime_cfg::ParameterObserver,
                              public utilx::Singleton<IODispatcherSubsystem*> {
public:
  IODispatcherSubsystem();
  virtual void start();
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

  using utilx::runtime_cfg::ParameterObserver::changeParameterEventHandler;
  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modifiedParameter);
private:
  smsc::core::threads::ThreadPool _threadsPool;
  size_t _stateMachinesCount;
};

}}}}

#endif
