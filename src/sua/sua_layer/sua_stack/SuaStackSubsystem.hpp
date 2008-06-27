#ifndef __SUA_SUALAYER_SUASTACK_SUASTACKSUBSYSTEM_HPP__
# define __SUA_SUALAYER_SUASTACK_SUASTACKSUBSYSTEM_HPP__ 1

# include <vector>
# include <string>
# include <core/synchronization/EventMonitor.hpp>

# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/sua_layer/ApplicationSubsystem.hpp>
# include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
# include <sua/sua_layer/runtime_cfg/ParameterObserver.hpp>

namespace sua_stack {

class SuaStackSubsystem : public sua_layer::ApplicationSubsystem,
                          public runtime_cfg::ParameterObserver,
                          public utilx::Singleton<SuaStackSubsystem*> {
public:
  SuaStackSubsystem();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  using ParameterObserver::removeParameterEventHandler;

  virtual void changeParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                           const runtime_cfg::Parameter& modifiedParameter);
  virtual void addParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                        runtime_cfg::Parameter* addedParameter);
  virtual runtime_cfg::CompositeParameter*
  addParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                           runtime_cfg::CompositeParameter* addedParameter);

  void notifyLinkShutdownCompletion();
private:
  std::vector<communication::LinkId> _sgpLinkIds;
  smsc::core::synchronization::EventMonitor _allLinksShutdownMonitor;
  volatile int _establishedLinks;

  void extractAddressParameters(std::vector<std::string>* addrs,
                                runtime_cfg::CompositeParameter* nextParameter,
                                const std::string& paramName);
};

}

#endif
