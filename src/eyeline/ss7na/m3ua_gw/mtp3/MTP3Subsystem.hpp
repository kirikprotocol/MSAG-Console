#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MTP3SUBSYSTEM_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MTP3SUBSYSTEM_HPP__

# include <vector>
# include <string>

# include "core/synchronization/EventMonitor.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/ParameterObserver.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {

class MTP3Subsystem : public common::ApplicationSubsystem,
                      public utilx::runtime_cfg::ParameterObserver,
                      public utilx::Singleton<MTP3Subsystem*> {
public:
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  using ParameterObserver::removeParameterEventHandler;

  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modified_parameter);
  virtual void addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                        utilx::runtime_cfg::Parameter* added_parameter);
  virtual utilx::runtime_cfg::CompositeParameter*
  addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                           utilx::runtime_cfg::CompositeParameter* added_parameter);

  virtual void notifyLinkShutdownCompletion();

protected:
  MTP3Subsystem();
  friend class utilx::Singleton<MTP3Subsystem*>;

private:
  std::vector<common::LinkId> _sgpLinkIds;
  smsc::core::synchronization::EventMonitor _allLinksShutdownMonitor;
  volatile int _establishedLinks;

  void extractAddressParameters(std::vector<std::string>* addrs,
                                utilx::runtime_cfg::CompositeParameter* next_parameter,
                                const std::string& param_name);

  void configurePoints(utilx::runtime_cfg::RuntimeConfig& rconfig);
  void fillInRoutingTable(utilx::runtime_cfg::RuntimeConfig& rconfig);
  void activateLinksToSGP(utilx::runtime_cfg::RuntimeConfig& rconfig);
};

}}}}

#endif
