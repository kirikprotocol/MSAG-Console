#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__

# include <sys/types.h>
# include "core/threads/ThreadPool.hpp"

# include "eyeline/utilx/runtime_cfg/ParameterObserver.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class IODispatcherSubsystem : public common::ApplicationSubsystem,
                              public utilx::runtime_cfg::ParameterObserver {
public:
  IODispatcherSubsystem::IODispatcherSubsystem(ConnectMgr& c_mgr)
    : ApplicationSubsystem("IODispatcherSubsystem", "io_dsptch"),
      _stateMachinesCount(0), _cMgr(c_mgr)
  {}

  virtual void start();
  virtual void stop();

protected:
  using utilx::Subsystem::initialize;
  void initialize();

  size_t _stateMachinesCount;

private:
  smsc::core::threads::ThreadPool _threadsPool;
  ConnectMgr& _cMgr;
};

}}}}

#endif