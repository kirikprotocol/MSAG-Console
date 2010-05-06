#ifndef __EYELINE_SS7NA_SUAGW_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_IODISPATCHER_IODISPATCHERSUBSYSTEM_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IODispatcherSubsystem.hpp"
# include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace io_dispatcher {

class IODispatcherSubsystem : public common::io_dispatcher::IODispatcherSubsystem,
                              public utilx::Singleton<IODispatcherSubsystem*> {
public:
  using IODispatcherSubsystem::initialize;
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

private:
  ConnectMgr& getInitedConnectMgr() {
    ConnectMgr::init();
    return ConnectMgr::getInstance();
  }

  IODispatcherSubsystem()
  : common::io_dispatcher::IODispatcherSubsystem(getInitedConnectMgr())
  {}
  friend class utilx::Singleton<IODispatcherSubsystem*>;
};

}}}}

#endif
