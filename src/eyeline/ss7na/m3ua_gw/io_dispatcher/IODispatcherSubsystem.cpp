#include "eyeline/utilx/strtol.hpp"
#include "IODispatcherSubsystem.hpp"
#include "ConnectMgr.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace io_dispatcher {

void
IODispatcherSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  common::io_dispatcher::IODispatcherSubsystem::initialize();
  utilx::runtime_cfg::Parameter& stateMachinesCount = rconfig.find<utilx::runtime_cfg::Parameter>("config.state_machines_count");
  _stateMachinesCount = stateMachinesCount.getIntValue();

  unsigned reconnectInterval = 1;
  utilx::runtime_cfg::Parameter& reconnectIntervalParam = rconfig.find<utilx::runtime_cfg::Parameter>("config.reconnect_interval");
  reconnectInterval = reconnectIntervalParam.getIntValue();
  if ( reconnectInterval == 0 && errno )
    throw smsc::util::Exception("IODispatcherSubsystem::initialize::: invalid value of config.reconnect_interval parameter=[%s]",
                                reconnectIntervalParam.getIntValue());

  ConnectMgr& connMgr = getInitedConnectMgr();
  common::io_dispatcher::Reconnector::getInstance().initialize(&connMgr, reconnectInterval);
}

}}}}
