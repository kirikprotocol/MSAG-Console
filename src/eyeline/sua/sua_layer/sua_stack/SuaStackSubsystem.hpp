#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_SUASTACKSUBSYSTEM_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_SUASTACKSUBSYSTEM_HPP__

# include <vector>
# include <string>
# include <core/synchronization/EventMonitor.hpp>

# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/ApplicationSubsystem.hpp>
# include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
# include <eyeline/utilx/runtime_cfg/ParameterObserver.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class SuaStackSubsystem : public sua_layer::ApplicationSubsystem,
                          public utilx::runtime_cfg::ParameterObserver,
                          public utilx::Singleton<SuaStackSubsystem*> {
public:
  SuaStackSubsystem();
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  using ParameterObserver::removeParameterEventHandler;

  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modifiedParameter);
  virtual void addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                        utilx::runtime_cfg::Parameter* addedParameter);
  virtual utilx::runtime_cfg::CompositeParameter*
  addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                           utilx::runtime_cfg::CompositeParameter* addedParameter);

  void notifyLinkShutdownCompletion();
private:
  std::vector<communication::LinkId> _sgpLinkIds;
  smsc::core::synchronization::EventMonitor _allLinksShutdownMonitor;
  volatile int _establishedLinks;

  void extractAddressParameters(std::vector<std::string>* addrs,
                                utilx::runtime_cfg::CompositeParameter* nextParameter,
                                const std::string& paramName);
};

}}}}

#endif
