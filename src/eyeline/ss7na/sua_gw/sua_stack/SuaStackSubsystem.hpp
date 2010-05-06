#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_SUASTACKSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_SUASTACKSUBSYSTEM_HPP__

# include <vector>
# include <string>

# include "core/synchronization/EventMonitor.hpp"

# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class SuaStackSubsystem : public common::ApplicationSubsystem,
                          public utilx::Singleton<SuaStackSubsystem*> {
public:
  SuaStackSubsystem();
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();

  virtual void notifyLinkShutdownCompletion();

private:
  std::vector<common::LinkId> _sgpLinkIds;
  smsc::core::synchronization::EventMonitor _allLinksShutdownMonitor;
  volatile int _establishedLinks;

  void extractAddressParameters(std::vector<std::string>* addrs,
                                utilx::runtime_cfg::CompositeParameter* next_parameter,
                                const std::string& param_name);
};

}}}}

#endif
