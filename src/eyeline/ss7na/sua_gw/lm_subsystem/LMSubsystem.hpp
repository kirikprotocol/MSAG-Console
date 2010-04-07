#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMSUBSYSTEM_HPP__

# include <netinet/in.h>
# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
# include "eyeline/utilx/runtime_cfg/ParameterObserver.hpp"

# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/UserInteractionProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class LMSubsystem : public common::ApplicationSubsystem,
                    public utilx::runtime_cfg::ParameterObserver,
                    public utilx::Singleton<LMSubsystem*> {
public:
  LMSubsystem();

  virtual void start();
  virtual void stop();
  virtual void waitForCompletion();

  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

private:
  UserInteractionProcessor* _userInteractionProcessor;
  std::string _host;
  in_port_t _port;
};

}}}}

#endif
