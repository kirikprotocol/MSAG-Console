#ifndef __SUA_SUALAYER_SUASTACK_SUASTACKSUBSYSTEM_HPP__
# define __SUA_SUALAYER_SUASTACK_SUASTACKSUBSYSTEM_HPP__ 1

# include <vector>
# include <string>
# include <sua/utilx/Subsystem.hpp>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>
# include <util/config/ConfigView.h>
# include <logger/Logger.h>
# include <core/synchronization/EventMonitor.hpp>

namespace sua_stack {

class SuaStackSubsystem : public utilx::Subsystem,
                          public utilx::Singleton<SuaStackSubsystem*> {
public:
  SuaStackSubsystem();
  virtual void start();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);
  virtual const std::string& getName() const;
  virtual void waitForCompletion();
  void notifyLinkShutdownCompletion();
private:
  std::string _name;
  smsc::logger::Logger *_logger;
  std::vector<communication::LinkId> _sgpLinkIds;
  smsc::core::synchronization::EventMonitor _allLinksShutdownMonitor;
  volatile int _establishedLinks;

  void extractAddressParameters(std::vector<std::string>* addrs,
                                const runtime_cfg::CompositeParameter* nextParameter,
                                const std::string paramName);
};

}

#endif
