#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMSUBSYSTEM_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMSUBSYSTEM_HPP__

# include <netinet/in.h>
# include <string>
# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/ApplicationSubsystem.hpp>
# include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
# include <sua/sua_layer/runtime_cfg/ParameterObserver.hpp>
# include <sua/sua_layer/lm_subsystem/UserInteractionProcessor.hpp>

namespace lm_subsystem {

class LMSubsystem : public sua_layer::ApplicationSubsystem,
                    public runtime_cfg::ParameterObserver,
                    public utilx::Singleton<LMSubsystem*> {
public:
  LMSubsystem();

  virtual void start();
  virtual void stop();
  virtual void waitForCompletion();

  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);

private:
  UserInteractionProcessor* _userInteractionProcessor;
  std::string _host;
  in_port_t _port;
};

}

#endif
